import axios, {AxiosInstance, AxiosError, AxiosResponse, AxiosRequestConfig} from 'axios';
import {get_token} from "../api/token";

const service: AxiosInstance = axios.create({
    baseURL:"https://www.huwai.pro",
    //baseURL: "http://127.0.0.1:8088",
    timeout: 5000
});

service.interceptors.request.use(
    (config: AxiosRequestConfig) => {
        if(config.url?.indexOf("/city/list") === -1) {
            const tokenData = get_token()
            if (tokenData != null) {
                config.headers = {
                    "Authorization": tokenData.token
                }
            }
        }
        return config;
    },
    (error: AxiosError) => {
        console.log(error);
        return Promise.reject();
    }
);

service.interceptors.response.use(
    (response: AxiosResponse) => {
        if (response.status === 200) {
            return response;
        } else {
            Promise.reject();
        }
    },
    (error: AxiosError) => {
        console.log(error);
        return Promise.reject();
    }
);

// @ts-ignore
export default service;
