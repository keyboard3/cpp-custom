console.log("hello world from jsEngine");
const rootDom = {
  width: 800,
  height: 800,
  children: [
    {
      x: 100,
      y: 20,
      width: 100,
      height: 40,
      fontSize: 14,
      paddingLeft: 20,
      innerText: "mali",
      background: "magenta",
      color: "black",
      onClick: function () {
        this.background = "yellow";
        this.color = "black";
        setRootDom(rootDom);
      }
    }, {
      x: 100,
      y: 100,
      width: 100,
      height: 40,
      fontSize: 14,
      paddingLeft: 20,
      background: "dkgray",
      color: "white",
      innerText: "World",
      onClick: function () {
        this.background = "red";
        this.color = "black";
        setRootDom(rootDom);
      }
    }
  ],
  onClick: function () {
    console.log("根节点 点击触发")
  }
}
setRootDOM(rootDom);