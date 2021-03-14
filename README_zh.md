# Treasure house(Th)

Treasure house 是一个开源的C程序库集合

我们使用

* cmake
* googletest

如果你使用 `Debian/Ubuntu` 等 `apt` 系Linux发行版，你可以通过下面的命令安装

```shell
sudo apt install cmake libgtest-dev
```

构建指令如下

```shell
rm build/ -rf && cmake -S . -B build
cmake --build build
```

执行`build/test/test_all` 即可运行测试套件