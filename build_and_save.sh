#!/bin/bash

docker build  --no-cache -t archesscube .

docker save -o ARChessCube.tar archesscube

echo "Образ сохранен в файл ARChessCube.tar"