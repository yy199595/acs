
const TokenKey = 'Authorization'
import request from '../utils/request';
interface TokenData {
    token : string
    tick : number,
}

interface AdminInfo {
    name : string,
    login_ip : string,
    city_name : string,
    permission : number,
    login_time : number,
    create_time : number
}

export function get_token() : TokenData | null {
    const token = localStorage.getItem(TokenKey)
    if(token) {
        const now = new Date().getTime() / 1000
        const tokenData = JSON.parse(token)
        if(tokenData.tick >= now) {
            return tokenData
        }
        localStorage.removeItem(TokenKey)
    }
    return null
}

export function add_token(token : TokenData) {
    localStorage.setItem(TokenKey, JSON.stringify(token))
}


export function is_admin() {
    try {
        const tokenInfo = get_token()
        if(tokenInfo == null)
        {
            return false;
        }
        const strArray = tokenInfo.token.split(".")
        const jsonData = JSON.parse(atob(strArray[1]))
        console.log(jsonData.p)
        return  jsonData && jsonData.p == 100
    }
    catch (e) {
        return false;
    }
}

export function remove_token() {
    localStorage.removeItem(TokenKey)
}

export function delete_token() {
    localStorage.removeItem(TokenKey)
}

export function logout() {
    return  request({
        "method" :"GET",
        "url" :"/admin/logout"
    })
}
