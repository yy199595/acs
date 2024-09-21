
export function format_time(row, column, cellValue) {
    const timestamp = new Date(cellValue * 1000); // 将时间戳转换为Date对象
     // 使用toLocaleString方法格式化时间字符串
    //let days = ["星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"];
    //return t + "(" + days[timestamp.getDay()] + ")"
    return timestamp.toLocaleString();
}


export function format_size(row, column, cellValue){
    const MB = 1024 * 1024;
    const GB = 1024 * 1024 * 1024
    if(cellValue >= GB) {
        const num = cellValue / GB
        return num.toFixed(3) + "G";
    }
    if (cellValue >= MB) {
        const num = cellValue / MB
        return num.toFixed(3) + "M";
    } else if (cellValue >= 1024) {
        const num = cellValue / 1024
        return num.toFixed(3) + "K";
    }
    return cellValue + "B"
}

// ZhuChuang : 8, //主创
//     ShangJia : 9, //商家
//     ZhuLiRen : 10, // 主理人
//     ZhuGuan : 50, //主管
//     Admin : 100  //管理员


export function format_permiss(row, column, cellValue) {
    console.log(cellValue)
    switch(cellValue)
    {
        case 0:
            return "用户";
        case 8:
            return "主创"
        case 9:
            return "商家"
        case 10:
            return "主理人";
        case 20:
            return "普通管理员"
        case 100:
            return "超级管理员"
    }
    return "未知"
}


