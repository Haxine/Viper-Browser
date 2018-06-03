# Viper Browser

![Coverity Scan Build Status](https://scan.coverity.com/projects/14853/badge.svg?flat=1 "Coverity Scan Build Status")

A lightweight web browser built with the Qt framework and QtWebEngine

Licensed under GPLv3

# Features

* Built-in AdBlock Plus support, with uBlock Origin filter compatibility (mostly compatible)
* Greasemonkey-style UserScript support
* Multi-tab style web browser
* Ability to customize the user agent
* Support for manual cookie addition / removal / modifications
* Toggle JavaScript on or off in settings
* Compatible with Pepper Plugin API

# Building

The browser software uses the qmake build system. The project can be built with QtCreator or through the command line by running 'qmake-qt5' or 'qmake', followed by 'make'

# Thanks

This project is possible thanks to the work of others, including those involved in the following projects:

* Qt Framework
* PDF.js (license in file PDFJS-APACHE-LICENSE)
* Arora QT Browser (licenses in files LICENSE.GPL2, LICENSE.LGPL3)
* Qt Tab Browser example (From Qt 5.5 Webkit example archive)
* Code Editor example (From Qt examples, license in file CodeEditor-LICENSE-BSD)
* Qupzilla - for parts of AdBlockPlus implementation (license in file GPLv3)
* Otter Browser - for parts of AdBlockPlus implementation (license in file GPLv3)
