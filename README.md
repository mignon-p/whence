Web browsers often use [extended attributes][1] to store the URL that
a file was downloaded from.  Sometimes I have a file lying around and
want to know where it was downloaded from, so I wrote the `whence`
command as an easy way to access this information on several major
platforms (FreeBSD, Linux, MacOS, and Windows).

## Usage

```
Usage: whence [OPTIONS] FILE ...

  -j, --json                  Print results in JSON format.
  -h, --help                  Print this message and exit.
  -v, --version               Print the version number of whence and exit.
```

## Example

```
bash$ whence wget-1.20.tar.gz
wget-1.20.tar.gz:
  URL         https://ftp.gnu.org/gnu/wget/wget-1.20.tar.gz
  Referrer    https://ftp.gnu.org/gnu/wget/
  Application Firefox
  Date        Sun Jun  7 11:30:18 PDT 2020
```

## Download and install

### Pre-built binaries

For release [0.9](https://github.com/ppelleti/whence/releases/tag/0.9):

* [FreeBSD, arm64](https://github.com/ppelleti/whence/releases/download/0.9/whence-0.9-freebsd-arm64.tar.bz2)
* [Linux, x86\_64](https://github.com/ppelleti/whence/releases/download/0.9/whence-0.9-linux-x86_64.tar.bz2)
* [MacOS, x86\_64](https://github.com/ppelleti/whence/releases/download/0.9/whence-0.9-macos-x86_64.tar.bz2)
* [Windows, x86\_64](https://github.com/ppelleti/whence/releases/download/0.9/whence-0.9-windows-x86_64.zip)

### Building from source

Run `./build.sh` to build.

On Windows, MinGW is assumed.  I haven't attempted to get it working
with MSVC.

On MacOS, [SQLite 3][8] is required, but it should already be present
as part of the operating system.

### Installation

On Windows, copy `whence.exe` to a directory on your `PATH`.

On other platforms, copy `whence` to `/usr/local/bin` and
copy `whence.1` to `/usr/local/share/man/man1`.

## Extended Attributes

### Linux and FreeBSD

On Linux and FreeBSD, the following extended attributes from
[Common Extended Attributes][2] are used:

* `user.xdg.origin.url`
* `user.xdg.referrer.url`
* `user.xdg.origin.email.from`
* `user.xdg.origin.email.subject`
* `user.xdg.origin.email.message-id`
* `user.xdg.publisher`

### MacOS

On MacOS, the following extended attributes are used:

* `com.apple.metadata:kMDItemWhereFroms`
* `com.apple.quarantine`

`com.apple.metadata:kMDItemWhereFroms` is an array of strings, stored
as a binary property list.  The array contains two strings (URL,
Referrer) for web pages, and three strings (From, Subject, Message-ID)
for email messages.

`com.apple.quarantine` is
[a string containing four fields, separated by semicolons][3].  The
fourth field is a UUID which is a key into the SQLite database
`~/Library/Preferences/com.apple.LaunchServices.QuarantineEventsV2`.
This database can be used to look up the URL and Referrer if
`com.apple.metadata:kMDItemWhereFroms` is absent.

### Windows

On Windows, the [NTFS alternate data stream][4] named
[Zone.Identifier][5] is used.  The contents of the stream look like
this:

```
[ZoneTransfer]
ZoneId=3
ReferrerUrl=https://mirrors.ocf.berkeley.edu/gnu/make/
HostUrl=https://mirrors.ocf.berkeley.edu/gnu/make/make-4.2.tar.bz2
```

The `ZoneId` number is mapped to a name by looking in
[the registry][6].

## License

`whence` is distributed under the terms of the [MIT License][7].

## Change Log

### 0.9.1 (unreleased)

* Support XDG attributes on MacOS.
* Support `com.apple.metadata:kMDItemDownloadedDate` on MacOS.
* Generate correct Unicode escapes in JSON strings.
* Support Unicode filenames on Windows.
* Added `install-whence.sh` script for convenience on UNIX.

### 0.9

* Initial release.

[1]: https://en.wikipedia.org/wiki/Extended_file_attributes
[2]: https://www.freedesktop.org/wiki/CommonExtendedAttributes/
[3]: https://eclecticlight.co/2017/12/11/xattr-com-apple-quarantine-the-quarantine-flag/
[4]: http://www.flexhex.com/docs/articles/alternate-streams.phtml
[5]: https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-fscc/6e3f7352-d11c-4d76-8c39-2516a9df36e8
[6]: https://support.microsoft.com/en-us/help/182569/internet-explorer-security-zones-registry-entries-for-advanced-users
[7]: LICENSE
[8]: https://sqlite.org/
