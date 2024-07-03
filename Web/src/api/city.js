
import request from '../utils/request';
import axios from "axios";
export function query_city_list() {
    return request({
            method: "GET",
            url: "/city/all"
        }
    )
}

export function open_city(code) {
    return request({
        method : "GET",
        url : "/city/open?code=" + code
    })
}

export function query_table(tab, page) {
    return request({
        method : "GET",
        url : "/table_mgr/find",
        params : { tab, page }
    })
}

export function delete_table(tab) {
    return request({
        method : "GET",
        url : "/table_mgr/delete",
        params : { tab }
    })
}

export async function get_city_list() {
    const data = localStorage.getItem("city_list")
    if(data) {
        return JSON.parse(data)
    }
    const response = await axios.get("https://yy-server-log.oss-cn-beijing.aliyuncs.com/city.json")
    if(response.data)
    {
        const json = JSON.stringify(response.data)
        localStorage.setItem("city_list", json)
        return response.data
    }
    return null
}
