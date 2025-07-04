#!/bin/bash

git add .
git commit -am "Updating libcs2"
git push
cd ..
cpkg compress libcs2
mv dist/libcs2.zip guille-ux.github.io
cd guille-ux.github.io
git add .
git commit -am "Updating libcs2"
git push
