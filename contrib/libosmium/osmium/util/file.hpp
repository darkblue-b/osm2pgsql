#ifndef OSMIUM_UTIL_FILE_HPP
#define OSMIUM_UTIL_FILE_HPP

/*

This file is part of Osmium (http://osmcode.org/libosmium).

Copyright 2013-2017 Jochen Topf <jochen@topf.org> and others (see README).

Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

*/

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <string>
#include <system_error>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#  ifndef WIN32_LEAN_AND_MEAN
#   define WIN32_LEAN_AND_MEAN // Prevent winsock.h inclusion; avoid winsock2.h conflict
#  endif
# include <io.h>
# include <windows.h>
#endif

#ifndef _MSC_VER
# include <unistd.h>
#endif

#include <osmium/util/cast.hpp>

namespace osmium {

    namespace util {

        /**
         * Get file size.
         * This is a small wrapper around a system call.
         *
         * @param fd File descriptor
         * @returns file size
         * @throws std::system_error If system call failed
         */
        inline size_t file_size(int fd) {
#ifdef _MSC_VER
            // Windows implementation
            // https://msdn.microsoft.com/en-us/library/dfbc2kec.aspx
            const auto size = ::_filelengthi64(fd);
            if (size == -1L) {
                throw std::system_error{errno, std::system_category(), "Could not get file size"};
            }
            return size_t(size);
#else
            // Unix implementation
            struct stat s;
            if (::fstat(fd, &s) != 0) {
                throw std::system_error{errno, std::system_category(), "Could not get file size"};
            }
            return size_t(s.st_size);
#endif
        }

        /**
         * Get file size.
         * This is a small wrapper around a system call.
         *
         * @param name File name
         * @returns file size
         * @throws std::system_error If system call failed
         */
        inline size_t file_size(const char* name) {
#ifdef _MSC_VER
            // Windows implementation
            // https://msdn.microsoft.com/en-us/library/14h5k7ff.aspx
            struct _stat64 s;
            if (::_stati64(name, &s) != 0) {
                throw std::system_error{errno, std::system_category(), std::string{"Could not get file size of file '"} + name + "'"};
            }
#else
            // Unix implementation
            struct stat s;
            if (::stat(name, &s) != 0) {
                throw std::system_error{errno, std::system_category(), std::string{"Could not get file size of file '"} + name + "'"};
            }
#endif
            return size_t(s.st_size);
        }

        /**
         * Get file size.
         * This is a small wrapper around a system call.
         *
         * @param name File name
         * @returns file size
         * @throws std::system_error If system call failed
         */
        inline size_t file_size(const std::string& name) {
            return file_size(name.c_str());
        }

        /**
         * Resize file.
         * Small wrapper around ftruncate(2) system call.
         *
         * @param fd File descriptor
         * @param new_size New size
         * @throws std::system_error If ftruncate(2) call failed
         */
        inline void resize_file(int fd, size_t new_size) {
#ifdef _WIN32
            // https://msdn.microsoft.com/en-us/library/whx354w1.aspx
            if (::_chsize_s(fd, static_cast_with_assert<__int64>(new_size)) != 0) {
#else
            if (::ftruncate(fd, static_cast_with_assert<off_t>(new_size)) != 0) {
#endif
                throw std::system_error{errno, std::system_category(), "Could not resize file"};
            }
        }

        /**
         * Get the page size for this system.
         */
        inline size_t get_pagesize() {
#ifdef _WIN32
            // Windows implementation
            SYSTEM_INFO si;
            GetSystemInfo(&si);
            return si.dwPageSize;
#else
            // Unix implementation
            return size_t(::sysconf(_SC_PAGESIZE));
#endif
        }

        /**
         * Get current offset into file.
         *
         * @param fd Open file descriptor.
         * @returns File offset or 0 if it is not available.
         */
        inline size_t file_offset(int fd) {
#ifdef _MSC_VER
            // https://msdn.microsoft.com/en-us/library/1yee101t.aspx
            auto offset = _lseeki64(fd, 0, SEEK_CUR);
#else
            auto offset = ::lseek(fd, 0, SEEK_CUR);
#endif
            if (offset == -1) {
                return 0;
            }
            return size_t(offset);
        }

        /**
         * Check whether the file descriptor refers to a TTY.
         */
        inline bool isatty(int fd) {
#ifdef _MSC_VER
            // https://msdn.microsoft.com/en-us/library/f4s0ddew.aspx
            return _isatty(fd) != 0;
#else
            return ::isatty(fd) != 0;
#endif
        }

    } // namespace util

} // namespace osmium

#endif // OSMIUM_UTIL_FILE_HPP
