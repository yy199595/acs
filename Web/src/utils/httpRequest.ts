
import request from "./request";

export namespace httpRequest
{
    export function GET(url : string) {
        return request({
            url : url,
            method : "GET"
        })
    }

    export function POST(url : string, data : any) {
        return request({
            url : url,
            method : "POST",
            data : data
        })
    }
}