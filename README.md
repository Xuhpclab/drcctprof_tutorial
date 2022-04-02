# drcctprof_tutorial

This is a framework used for A hands-on lab: developing the first DrCCTProf client tool on ARM in [DrCCTProf Tutorial](https://www.xperflab.org/drcctprof/tutorial).

## Requirement

Try to implement a DrCCTProf client to count each **instruction**<sup>*</sup> with unique call stacks.

> `*` `Instructions of the same PC (program counter) may appear in different call paths.`


You just need to finish the function of [void InsCount(int32_t opaqueHandle)](https://github.com/Xuhpclab/drcctprof_tutorial/blob/main/src/client.cpp#L42-L55)
:

1. get the current context handle
   Tip: use API 
   ```c
    context_handle_t drcctlib_get_context_handle(void *drcontex, int32_t opaqueHandle)
   ```
2. get the executed times of the current context handle
    Tip: use *ctxt_hndl_exec_num_array*  to get and store every context handle's executed times

3. add 1 for the executed times and store it in the array

## Build

Use the following commands to get source code and build:

```console
$ git clone --recurse https://github.com/Xuhpclab/drcctprof_tutorial.git
```

```console
$ ./build.sh
```

## Run

After you finish the implementation, run the following commands to rebuild your source code and run the tool to profile the test application:

```console
$ ./run.sh
```

The process will generate two profiles, one is a text file, and one is a .drcctprof file. You can directly open the .drcctprof file in VSCode if you have installed our [Viewer extension](https://marketplace.visualstudio.com/items?itemName=xuhpclib-easyview.easyview).

### *Expected screenshot result*

> <div align=center><img src="screenshot.png"/> </div>