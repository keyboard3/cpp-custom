import { setTimeout, clearTimeout } from "os"
Promise.resolve()
    .then(()=>{
        console.log("1：微任务执行1");
        setTimeout(()=>console.log("宏任务2"),1000);
    });
setTimeout(() => {
    console.log("宏任务1：");
    Promise.resolve().then(()=>console.log("2：微任务1")).then(()=>console.log("2：微任务2"))
    let timeId = setTimeout(() => console.log("libuv 第二个延时，应该被取消"), 1000);
    clearTimeout(timeId);
}, 0);