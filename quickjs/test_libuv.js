import { setTimeout, clearTimeout } from "libuv.so"
setTimeout(() => {
    console.log("libuv timeout 调用完成");
    let timeId = setTimeout(() => console.log("libuv 第二个延时，应该被取消"), 1000);
    clearTimeout(timeId);
}, 1000);