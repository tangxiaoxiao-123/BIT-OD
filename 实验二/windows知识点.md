TCHAR：程序编译为 ANSI， TCHAR 就是相当于 CHAR；当程序编译为 UNICODE， TCHAR 就相当于 WCHAR

LPCTSTR:用来表示你的[字符](https://baike.baidu.com/item/字符/4768913)是否使用UNICODE, 如果你的程序定义了UNICODE或者其他相关的宏，那么这个[字符](https://baike.baidu.com/item/字符/4768913)或者字符串将被作为UNICODE字符串，否则就是标准的ANSI字符串。