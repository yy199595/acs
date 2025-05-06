
const TokenKey = 'Authorization'
import request from '../utils/request';
interface TokenData {
    token : string
    tick : number,
}

interface AdminInfo {
    name : string,
    token : string,
    exp_time : number,
    login_ip : string,
    city_name : string,
    permission : number,
    login_time : number,
    create_time : number
}

export namespace app {

    export function remove_user_info() {
        localStorage.removeItem("user_info")
    }

    export function get_user_info(): AdminInfo | null {
        const value = localStorage.getItem("user_info")
        console.log(value)
        if (value == null || value.length == 0) {
            return null;
        }
        return JSON.parse(value);
    }

    export function is_admin() : boolean {
        try {
            const userInfo = get_user_info();
            if(userInfo == null) {
                return false;
            }
            return userInfo.permission === 100;
        }
        catch (e) {
            return false;
        }
    }
}



export function logout() {
    return  request({
        "method" :"GET",
        "url" :"/admin/logout"
    })
}
