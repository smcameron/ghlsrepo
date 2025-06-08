
This is a little program that uses libcurl and cJSON to list out your
github repos.


I have quite a few github repos, enough that I don't remember them all
and enough that it's a pain in the ass to use the github website to
scroll through the many pages of them looking for the one I am am only
hazily recalling something about.

One day, I was thinking I should be able to get a list of github repos
from the command line, shouldn't I?  But I am not a web programmer, so
I don't know how to do that.  So I asked Google Gemini to write such a
program for me.  And it did.  This is that program.  Took a couple of
tries, but only a couple. Since it was written by machine, it is not
copyrightable, so I used the Unlicense to put it into the public domain.

Usage:

```
$ export GITHUB_USERNAME=your-user-name
$ export GITHUB_TOKEN=your-personal-access-token
$ ./ghlsrepos
```

To build this program:

First build and install cJSON:

```
$ git clone git@github.com:DaveGamble/cJSON.git
$ cd cJSON
$ make
$ sudo make install
```

Install libcurl, if you don't already have it:

```
$ apt-get install libcurl4-openssl-dev
```

Clone this repo and build it:

```
$ git clone git@github.com:smcameron/ghlsrepo.git
$ cd ghlsrepo
$ make
```

Run it:

```
$ export GITHUB_USER=your-user-name
$ export GITHUB_TOKEN=your-github-personal-access-token
$ ./ghlsrepo
```

