#!/bin/sh
until bin/gal_rest 192.168.1.244 80; do
    echo "Server 'myserver' crashed with exit code $?.  Respawning.." >&2
done
