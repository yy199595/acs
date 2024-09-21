import request from '../utils/request';

export const RequestAllList = () => {
    return request({
        method: "get",
        url: "./admin/all_info",
    });
}

export const RequestList = (id : number) => {
    return request({
        method: "get",
        url: "./admin/info?id=" + id,
    });
}

export const RequestStopServer = async (id: number)=> {
    return request({
        method: "get",
        url : "./admin/stop",
        params : { id : id }
    })
}

export const RequestRpcList = (data: any) => {
    return request({
        method: "get",
        url: "./admin/rpc",
        params: data
    })
}

export const RequestHttpList = (data : any) => {
    return request({
        method: "get",
        url: "./admin/http",
        params : data
    })
}

export const RequestHotfix = (id : number) => {
    return request({
        method: "get",
        url: "./admin/hotfix?id=" + id
    })
}

export const RequestPing = (id : number) => {
    return request({
        method: 'get',
        url: './admin/ping?id=' + id
    })
}

export const RequestLogList = ()=> {
    return request({
        method : "get",
        url : "./log/list"
    })
}

export const RequestDownloadLog = (file : string)=> {
    return request({
        method : "GET",
        url : "./log/download?file=" + file,
    })
}

export const RequestGetMenus = () => {
    return request({
        method : "GET",
        url : "/admin/menu"
    })
}

export const RequestCards = () => {
    return request({
        method : "GET",
        url : "/api/card_list"
    })
}

export const CreateVipCard = (data : any)=> {
    return request({
        method : "POST",
        url : "/vip/create",
        data : data,
        headers: {"content-type": "application/json"}
    })
}

export const DeleteVipCard = (id : number) => {
    return request({
        method : "GET",
        url : "/vip/del",
        params : { id : id}
    })
}

export const RequestOrderList = (page : number, status : number, id : string, city : number, product_id : number, type : number) => {
    return request({
        method : "GET",
        url : "/order/list",
        params : { page, status, id, city, product_id, type}
    })
}

export const RefundOrder = (order_id : string) => {
    return request({
        method : "GET",
        url : "/order/web_refund",
        params : { id : order_id }
    })
}
