# ossclient ##
用来做上传资源以及资源版本控制的独立程序
资源存储使用阿里云oss
上传多个文件以及文件夹时用;将字符串分隔

# AssetUpdate ##
该模块是用来下载资源的，支持断点续传，数据结构跟上传工具要一致才能一起使用

# package.rar ##
这个是打包好的上传工具，配置好 *ossclientconfig/oss_account.json* 文件就能直接使用

## 编译 ##

将 ossclient文件夹 放到 *Engine\Source\Programs* 目录下，重新生成解决方案编译即可
代码使用4.25编写
将这一级目录下的 ossclientconfig 文件夹放到 *Engine\Binaries\Win64* 目录下

## TODO ##

* 支持中文
* 进度条
* 以及一些细节的优化
* ...