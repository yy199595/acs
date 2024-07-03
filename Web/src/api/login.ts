import request from '../utils/request';

export interface LoginReqData {
    user : string,
    passwd : string
}

export const RequestLogin = async (req : LoginReqData) => {
    return request({
        method: "get",
        url: "./admin/login",
        params: req
    });
}

export const RequestOptRecord = async (page : number) => {
    return request({
        method: "get",
        url: "./record/list",
        params: { page : page},
    });
}

export const RequestAdminList = async (page : number) => {
    return request({
        method : "GET",
        url : "./admin/list",
        params : { page : page }
    })
}

export const RequestRegister = async (info : any) => {
    return request({
        method : "post",
        url : "./admin/register",
        data : info,
        headers: {"content-type": "application/json"}
    })
}
