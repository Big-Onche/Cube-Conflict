# Building Cube Conflict

## Linux

From the repository root:

```sh
make -C src -j"$(nproc)"
./run.sh
```

The build creates `bins/cc_client` and `bins/cc_server`. Official releases include Linux binaries; source-only checkouts can build them with the command above. Dependencies, distribution-specific package commands, server instructions, and troubleshooting are documented in [the Linux guide](../docs/linux.md).

## Windows

Install Code::Blocks with MinGW-w64, open `src/project/Cube Conflict.cbp`, and build the desired target. The Visual Studio project is also available under `src/project/`.

## macOS

The macOS build is experimental and is not part of the verified release path.

---

# Compiler Cube Conflict

Sous Linux, installez les dépendances décrites dans [le guide Linux](../docs/linux.md), puis lancez `make -C src -j"$(nproc)"` depuis la racine du dépôt. Les exécutables sont créés dans `bins/` et se lancent avec `./run.sh`.
