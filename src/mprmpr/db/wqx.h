

/*

0) 何为 REST?
   REST 为Representational State Transfer缩写，翻译为: 表现层状态转化.
   到底说的是啥啊(靠)?! 其实，它省略了主语. 整句话应该是: 资源表现层状态转化.
   那么问题来了, 什么是资源呢? 在Web中, 凡是可以识别, 可命名, 可访问并且被
   处理的实体都是资源.
   比如，html页面, 音频，视频，图片等等. 同时, 该资源被一个唯一的URI标识，并且
   使用http 的方法来执行操作. 总结: RESTful API 由下面几个约束:
   a. 资源
   b. 资源表现层
   c. 状态转化
   d. 统一接口

1) ISLI码 RESTful API 实战
   背景: 
       我们要开发一个管理ISLI码的RESTful API.
   步骤:
       a. 资源
       我们要抽象出资源的概念出来, 在我们的例子中, 我把ISLI码标识为资源.它的URL为:
       /api/mpr/islicode.

       b. 资源表现层
       所谓的表现层, 也可以理解为表述. 即资源的表述形式. 比如, HTML, XML, JSON, MP3, OGG...(其实就是HTTP服务的MIME).
       这里, 我选择使用 JSON形式.

       c. 状态转化
       由于我们是建立在http/https之上传输, 所以,每一次的操作都是无状态的. 其实,每个资源都会有一个状态,
       只是该状态保存在服务器端(的数据库中). 很自然的, 我们客户端必须有一种方法来改变服务器端的状态. 
       我们允许客户端创建一个ISLI code, 废除它, 访问它等等. 这些操作都让服务端中该ISLI code的状态发送转化.
  
       d. 统一接口
       我们后续的接口都要按照以上a,b,c三点来约束; 这样, 无论是使用, 维护, 还是开发这些接口的人一眼就知道这个API的框架.
   详解:
       a. 创建ISLI code
      Client -> Server: 客户端请求

	POST /api/mpr/islicode/ HTTP/1.1
	Accept: application/json
	Content-Length: 16
	Content-Type: application/json
	Host: localhost:9000

	{
	    "user_id": "1"
	}

      Server -> Client:　服务器回应

	HTTP/1.0 200 OK
	Content-Length: 64
	Content-Type: application/json

	{
	    "code": "00000011134566", 
	    "id": 1, 
	    "return_code": 0
	}

   总结:
       例子我就不一一写出了(我是使用vi编辑器, 不太好复制粘贴). 我这里总结一下HTTP的方法的语义.
       
       OPTIONS  获取该资源支持的HTTP 方法
       HEAD     获取某个资源返回的头信息
       GET      获取某个资源的信息
       POST     创建某个资源
       PUT      更新或者创建某个资源
       DELETE   删除某个资源

   误区:
       这里我也列出一些常犯的误区, 其实早年的时候我也踩坑, 目前泛媒阅读还有很多没有改正. 我们一致认为等系统稳定之后, 我们会重新规划我们的接口.
       0) 不可以在URL中使用动词
         /api/mpr/islicode/create
         /api/mpr/islicode/delete 这些都是不合法的
       1) 使用标准的http状态码表示执行状态
         200 表示成功
         404 表示资源不可用
         ...
       2) 接口版本说明
         有三种方法
         a. 保存在URL中:    https://reader.fanmei.com/api/v1/mpr/islicode
         b. 放在标准的头部: Accept: application/vnd.github.v3+json
         c. 放在自定义头部: mpr-api-version: 1
         第一种比较友好, 目前思赜们也在使用.
         第二种是 github使用的方式.
         第三种不推荐.
*/
