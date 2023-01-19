# muss
Mark Up Static Site

muss is a simple static site generator that aims to be minimal (
[KISS](https://en.wikipedia.org/wiki/KISS_principle) ), not do everything - it
just wants to solve the problem of html being hard to reuse on its own and
writing it for a blog is not reasonable ( markdown is much better ), it is
written in posix compliant C99 ( gods language :> )

## Features
* markdown with [lowdown](https://kristaps.bsd.lv/lowdown/) api
* basic css framework for styling, [picocss](https://picocss.com/)
* using c implementation of the [mustache](https://mustache.github.io/)
  template engine (import html snippets, do basic logic)
