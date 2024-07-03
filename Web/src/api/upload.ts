import request from '../utils/request';
import OSS from "ali-oss";

export const oss_upload = async (file: File) => {
    try {
        const type = file.type
        const result = await request({
            method: "GET",
            url: "oss/request?type=" + type,
        })
        const response = result.data
        if (response.code != 0) {
            return null
        }

        const client = new OSS({
            region: response.data.region,
            bucket: response.data.bucket,
            accessKeyId: response.data.key,
            accessKeySecret: response.data.secret,
        })
        const file_name = response.data.name
        const oss_result = await client.put(file_name, file)
        console.log(oss_result)
        return oss_result.url
    } catch (e) {
        console.error(e)
        return null
    }
}

export const api_upload = async (file: File) =>{
    try {
        const response = await request({
            method : "POST",
            url : "./res/upload",
            data : file
        })
        return response.data.url
    } catch (e) {
        return null
    }
}
