<template>
    <div class="login-wrap">
        <div class="ms-login">
            <div class="ms-title">服务器管理后台</div>
            <el-form :model="param" :rules="rules" ref="login" label-width="0px" class="ms-content">
                <el-form-item prop="username">
                    <el-input v-model="param.username" placeholder="username">
                        <template #prepend>
                            <el-button :icon="User"></el-button>
                        </template>
                    </el-input>
                </el-form-item>
                <el-form-item prop="password">
                    <el-input
                        type="password"
                        placeholder="password"
                        v-model="param.password"
                        @keyup.enter="submitForm"
                    >
                        <template #prepend>
                            <el-button :icon="Lock"></el-button>
                        </template>
                    </el-input>
                </el-form-item>
                <div class="login-btn">
                    <el-button type="primary" @click="submitForm">登录</el-button>
                </div>
            </el-form>
        </div>
    </div>
</template>

<script setup lang="ts">
import {ref, reactive} from 'vue';
import {useTagsStore} from '../store/tags';

import {useRouter} from 'vue-router';
import {ElMessage} from 'element-plus';
import type {FormInstance, FormRules} from 'element-plus';
import {Lock, User} from '@element-plus/icons-vue';
import {RequestLogin, LoginReqData} from "../api/login";
import {app} from "../api/token";

interface LoginInfo {
    username: string;
    password: string;
}

const router = useRouter();
const param = reactive<LoginInfo>({
    username: '',
    password: ''
});

const rules: FormRules = {
    username: [
        {
            required: true,
            message: '请输入用户名',
            trigger: 'blur'
        }
    ],
    password: [{required: true, message: '请输入密码', trigger: 'blur'}]
};

const login = ref<FormInstance>();
const submitForm = async () => {
    const login_req: LoginReqData =
        {
            user: param.username,
            passwd: param.password,
        }
    try {
        app.remove_user_info()
        const response = await RequestLogin(login_req)
        console.log(response.data)
        const info = response.data.data.info
        info.token = response.data.data.token
        if (response.data.code == 0) {
            localStorage.setItem("user_info", JSON.stringify(info))
            await router.push('/');
        } else {
            ElMessage.error(response.data.error)
        }
    } catch (e) {
        console.error(e)
        ElMessage.error("登陆失败,请检查网络")
    }

};

const tags = useTagsStore();
tags.clearTags();
</script>

<style scoped>
.login-wrap {
    display: flex;
    align-items: center;
    justify-content: center;
    width: 100%;
    height: 100vh;
    background: url(../assets/img/login-bg.jpg) center/cover no-repeat;
}

.ms-title {
    width: 100%;
    line-height: 50px;
    text-align: center;
    font-size: 20px;
    color: #000000;
    border-bottom: 1px solid #ddd;
}

.ms-login {
    position: absolute;
    left: 50%;
    top: 50%;
    width: 350px;
    margin: -190px 0 0 -175px;
    border-radius: 5px;
    background: rgba(255, 255, 255, 0.3);
    overflow: hidden;
}

.ms-content {
    padding: 30px 30px;
}

.login-btn {
    text-align: center;
}

.login-btn button {
    width: 100%;
    height: 36px;
    margin-bottom: 10px;
}

</style>
