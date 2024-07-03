
import request from '../utils/request';

export const RequestCityList = ()=> {
    return request({
        method: "get",
        url: "/city/list",
    });
}

export const RequestAccountList = (page: number, account: string, city_id : number) => {
    return request({
        method: "get",
        url: "./account_mgr/list",
        params: {page: page, open_id: account, city_id : city_id},
    });
}

export const RequestDeleteAccount = (open_id : string) => {
    return request({
        method: "get",
        url: "./account_mgr/del",
        params: {open_id: open_id},
    });
}

export const request_account_info = (user_id : string) => {
    return request({
        method: "get",
        url: "./account_mgr/find",
        params: {id: user_id},
    });
}

export const change_role_info = (info : any) => {
    return request({
        method: "post",
        url: "./account_mgr/change",
        data: info,
    });
}

export const request_user_info = (user_id : number)=> {
    return request({
        method: "get",
        url: "./user/find",
        params: {id: user_id},
    });
}
