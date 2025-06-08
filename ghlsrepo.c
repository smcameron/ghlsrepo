#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>

/* A struct to hold the data from the HTTP response (both body and headers) */
struct ResponseData {
  char *body;
  size_t body_size;
  char *headers;
  size_t headers_size;
};

/* Callback function to write response body into a memory buffer */
static size_t write_body_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct ResponseData *mem = (struct ResponseData *)userp;

	char *ptr = realloc(mem->body, mem->body_size + realsize + 1);
	if (ptr == NULL) {
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	mem->body = ptr;
	memcpy(&(mem->body[mem->body_size]), contents, realsize);
	mem->body_size += realsize;
	mem->body[mem->body_size] = 0;

	return realsize;
}

/* Callback function to write response headers into a memory buffer */
static size_t write_header_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct ResponseData *mem = (struct ResponseData *)userp;

	char *ptr = realloc(mem->headers, mem->headers_size + realsize + 1);
	if (ptr == NULL) {
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}

	mem->headers = ptr;
	memcpy(&(mem->headers[mem->headers_size]), contents, realsize);
	mem->headers_size += realsize;
	mem->headers[mem->headers_size] = 0;

	return realsize;
}


/* Function to parse the Link header and find the 'next' URL */
char* find_next_url(const char* link_header)
{
	if (link_header == NULL) {
		return NULL;
	}

	const char *next_str = "rel=\"next\"";
	char *next_ptr = strstr(link_header, next_str);
	if (next_ptr == NULL) {
		return NULL;
	}

	/* Find the start of the URL (search backwards for '<') */
	char *url_start = NULL;
	for (char *p = next_ptr; p > link_header; --p) {
		if (*p == '<') {
			url_start = p + 1;
			break;
		}
	}

	if (url_start == NULL) {
		return NULL;
	}

	/* Find the end of the URL (search forwards for '>') */
	char *url_end = strchr(url_start, '>');
	if (url_end == NULL) {
		return NULL;
	}

	size_t url_len = url_end - url_start;
	char *next_url = malloc(url_len + 1);
	if (next_url == NULL) {
		return NULL;
	}

	strncpy(next_url, url_start, url_len);
	next_url[url_len] = '\0';

	return next_url;
}

int main(__attribute__((unused)) int argc, __attribute__((unused)) char *argv[])
{
	CURL *curl_handle;
	CURLcode res;

	/* Get GitHub username and personal access token from environment variables */
	const char *github_username = getenv("GITHUB_USERNAME");
	const char *github_token = getenv("GITHUB_TOKEN");

	if (github_username == NULL || github_token == NULL) {
		fprintf(stderr, "Error: GITHUB_USERNAME and GITHUB_TOKEN environment variables must be set.\n");
		return 1;
	}

	curl_global_init(CURL_GLOBAL_ALL);
	
	/* Set up the authorization header (it's the same for all requests) */
	struct curl_slist *headers = NULL;
	char auth_header[256];
	snprintf(auth_header, sizeof(auth_header), "Authorization: token %s", github_token);
	headers = curl_slist_append(headers, auth_header);

	/* Initial URL to fetch */
	char *current_url = malloc(256);
	snprintf(current_url, 256, "https:/api.github.com/users/%s/repos?per_page=100", github_username);

	printf("Fetching repositories for %s...\n", github_username);

	do {
		struct ResponseData chunk;
		chunk.body = malloc(1);
		chunk.headers = malloc(1);
		chunk.body_size = 0;
		chunk.headers_size = 0;

		curl_handle = curl_easy_init();
		if (!curl_handle) {
			fprintf(stderr, "curl_easy_init() returned NULL.\n");
			return 1;
		}
		curl_easy_setopt(curl_handle, CURLOPT_URL, current_url);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_body_callback);
		curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
		curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, write_header_callback);
		curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void *)&chunk);
		curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
		curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);

		res = curl_easy_perform(curl_handle);

		if (res != CURLE_OK) {
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			free(current_url);
			current_url = NULL;
			break;
		}
		cJSON *json = cJSON_Parse(chunk.body);
		if (json == NULL) {
			const char *error_ptr = cJSON_GetErrorPtr();
			if (error_ptr != NULL)
				fprintf(stderr, "JSON Parse Error: %s\n", error_ptr);
		} else {
			cJSON *repo = NULL;
			cJSON_ArrayForEach(repo, json) {
				cJSON *name = cJSON_GetObjectItemCaseSensitive(repo, "name");
				if (cJSON_IsString(name) && (name->valuestring != NULL))
					printf("- %30s git@github.com:%s/%s.git\n",
						name->valuestring, github_username, name->valuestring);
			}
			cJSON_Delete(json);
		}

		/* Find the next URL and update for the next loop iteration */
		char* next_url_from_header = find_next_url(chunk.headers);
		free(current_url);
		current_url = next_url_from_header;

		curl_easy_cleanup(curl_handle);
		free(chunk.body);
		free(chunk.headers);
	} while (current_url != NULL);

	curl_slist_free_all(headers);
	curl_global_cleanup();

	return 0;
}

