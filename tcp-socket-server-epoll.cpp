/**
 * epoll 可以通过注册 callback 函数的方式，监听某个文件描述符发生变化，就会触发相应的事件
 * epoll_create 创建一个 epoll 对象，它的文件描述符对应了内核中的红黑树的结构
 * epoll_ctl 添加一个 Socket 时，向红黑树添加一个节点，这个节点存储了关心这个 Socket 变化事件的回调函数列表
 * 当 Socket 的事件列表发生变化可以找到挂的 epoll 对象，然后从 epoll 中找到回调并触发它
 **/