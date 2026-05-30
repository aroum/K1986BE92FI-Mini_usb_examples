#!/bin/bash
exec "$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)/build_all.sh" vcom "$@"
