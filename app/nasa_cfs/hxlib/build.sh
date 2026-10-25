#!/usr/bin/env bash
#
# hxlib 编译/清理脚本
#
#   ./build.sh           -> 编译 (make)
#   ./build.sh clean     -> 清理 (make clean)
#   ./build.sh rebuild   -> 先清理再编译
#
set -e
cd "$(dirname "$0")"

case "${1:-build}" in
    build)
        make
        ;;
    clean)
        make clean
        ;;
    rebuild)
        make clean
        make
        ;;
    *)
        echo "用法: $0 [build|clean|rebuild]"
        exit 1
        ;;
esac
