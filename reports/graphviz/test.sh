#!/bin/bash
dot -Tpng graph.gv > graph.png; xdg-open graph.png
dot -Tpng tree.gv > tree.png; xdg-open tree.png
dot -Tpng branch.gv > branch.png; xdg-open branch.png
