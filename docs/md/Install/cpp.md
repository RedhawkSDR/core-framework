# C++

Starting with version 3, REDHAWK is compiled at C++ 14.

Redhat/CentOS has [Devtoolset](https://access.redhat.com/documentation/en-us/red_hat_developer_toolset/9/pdf/user_guide/red_hat_developer_toolset-9-user_guide-en-us.pdf) to allow a host to have multiple C++ toolchains installed.
See the section 'Development Tools Setup' for scripts distributed with REDHAWK to help switch between these toolchains.

Older components are not necessary incompatible from a C++ ABI perspective.
To understand that aspect of compatibility, see the next section.


## C++ Language Standard Support

To ensure compatibility, it is important that your compiler has complete support for your C++ language standard.  For example, `g++` version 4.x.x has complete support for C++98/C++03, while `g++` version 5.x.x has complete support for C++11.  If object A was compiled with `g++` 4.8.5 at `-std=gnu++03` and object B was compiled with `g++` 5.1.0 at `-std=gnu++11`, then these objects are ABI compatible and can be linked.

As a counter-example, `g++` version 4.x.x does not have complete support for C++11.  If object A was compiled with `g++` 4.8.5 at `-std=gnu++11` and object B was compiled with `g++` 5.1.0 at `-std=gnu++11`, then these objects are ABI incompatible and should not be linked.

To determine whether a version of `g++` has complete support for a language standard, look [here](https://gcc.gnu.org/onlinedocs/libstdc++/manual/api.html).

REDHAWK is developed and tested using the C++14 standard and may not compile in C++17 or newer modes.  To configure GCC to use the C++14 standard, set the `CXXFLAGS` environment variable before building REDHAWK from source:
```bash
export CXXFLAGS="--std=gnu++14"
```

## Development Tools Setup

REDHAWK supports CentOS 7, for which the default C++ compiler is `g++` version 4.x.x, with complete support for C++98/C++03, but not for later versions.  REDHAWK requires a compiler with complete support for the C++14 standard.  For CentOS 7, that is provided by Software Collections (SCL), in the `devtoolset-<ver>-gcc-c++` rpm package.

The `devtoolset-<ver>-gcc-c++` rpm package installs to the `/opt/` directory a set of GNU software development tools such as `g++`, `gdb`, `ld`, and `autoconf`.  The use of these tools is enabled by prepending several environment PATH-like variables with `/opt/rh/devtoolset-<ver>`.  After completing the `./redhawk-install.sh` command below, these commands will be available to enable and disable them:
```bash
. $OSSIEHOME/bin/redhawk-devtoolset-enable.sh
. $OSSIEHOME/bin/redhawk-devtoolset-disable.sh
```

If you want these tools enabled automatically, add `redhawk-devtoolset-enable.sh` to `~/.bashrc`, or similar.

