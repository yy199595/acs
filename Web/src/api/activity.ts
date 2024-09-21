import request from '../utils/request';

export const pub_activity = (req: any) => {
    return request({
            method: "POST",
            url: "/activity/pub",
            data: req
        }
    )
}

export const change_activity = (req : any)=> {
    return request({
            method: "POST",
            url: "/activity/change",
            data: req
        }
    )
}

export const request_activity_list = (page : number, status : number, city : number) => {
    return request({
        method :"GET",
        url : "/activity/list?page=" + page + "&status=" + status + "&city=" + city
    })
}

export const request_delete_activity = (id : number) => {
    return request({
        method :"GET",
        url : "/activity/del?id=" + id
    })
}

export const request_city_list = ()=> {
    return request({
        method :"GET",
        url : "/city/list"
    })
}

export const request_city_info  = (code : number) => {
    return request({
        method :"GET",
        url : "/city/info?code=" + code
    })
}

export const request_club_list = (page : number)=> {
    return request({
        method :"GET",
        url : "/club_mgr/list?page=" + page
    })
}

export const dissolve_club = (id : number) => {
    return request({
        method :"GET",
        url : "/club_mgr/dissolve?club_id=" + id
    })
}
