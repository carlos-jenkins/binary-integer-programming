About
=====

Linear programming (LP, or linear optimization) is a mathematical method for
determining a way to achieve the best outcome (such as maximum profit or
lowest cost) in a given mathematical model for some list of requirements
represented as linear relationships.

If some or all of the unknown variables are required to be integers, then the
problem is called an integer programming (IP) or integer linear programming
(ILP) problem.  0-1 integer programming or binary integer programming (BIP) is
the special case of integer programming where variables are required to be 0 or
1 (rather than arbitrary integers).

This package includes an GUI application for input the model and an
implementation of the *Implicit Enumeration* algorithm.

This Software is written is C and uses GTK+ 3 for the GUI. It uses LaTeX to
render reports and Graphviz to draw the branches of problem tree.

![Binary Integer Programming GUI and reports](https://raw.github.com/carlos-jenkins/binary-integer-programming/master/media/wall.png "Binary Integer Programming GUI and reports")


How to build
============

Install dependencies:

```shell
sudo apt-get install build-essential texlive libgtk-3-dev graphviz
```

Then build and run:

```shell
cd download/dir
./configure
make
./bin/main
```


How to hack
===========

Install development Software:

```shell
sudo apt-get install glade geany git devhelp libgtk-3-doc
```

We use Kernel code standards except that identation is 4 spaces wide:

- http://www.kernel.org/doc/Documentation/CodingStyle

This package is developed with Gtk+ and glib:

- http://developer.gnome.org/glib/stable/
- http://developer.gnome.org/gtk3/stable/


License
=======

Copyright (C) 2012 Carolina Aguilar <caroagse@gmail.com>  
Copyright (C) 2012 Carlos Jenkins <carlos@jenkins.co.cr>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

