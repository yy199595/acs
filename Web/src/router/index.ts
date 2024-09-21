import { createRouter, createWebHashHistory, RouteRecordRaw } from 'vue-router';
import Home from '../views/home.vue';
import {get_token} from "../api/token";

const routes: RouteRecordRaw[] = [
    {
        path: '/',
        redirect: '/welcome',
    },
    {
        path: '/',
        name: 'Home',
        component: Home,
        children: [
            {
                path: '/welcome',
                name: 'welcome',
                meta: {
                    title: '系统首页',
                },
                component: () => import('../views/welcome.vue'),
            },
            {
                path:"/dashboard",
                name:"dashboard",
                meta: {
                    title: "服务器管理"
                },
                component: ()=>  import('../views/dashboard.vue')
            },
            {
                path:"/mongodb",
                name:"mongodb",
                meta: {
                    title: "MongoDB管理"
                },
                component: ()=>  import('../views/mongodb.vue')
            },
            {
                path : "/account",
                name : "account",
                meta : {
                    title : "用户列表"
                },
                component:()=> import("../views/account.vue")
            },
            {
                path : "/mch_list",
                name : "mch_list",
                meta : {
                    title : "部落列表"
                },
                component:()=> import("../views/club_info.vue")
            },
            {
                path : "/role",
                name : "role",
                meta : {
                    title : "用户信息"
                },
                component:()=> import("../views/role.vue")
            },
            {
                path : "/vip",
                name : "vip",
                meta : {
                    title : "会员卡管理"
                },
                component:()=> import("../views/vip.vue")
            },
            {
                path : "/order",
                name : "order",
                meta : {
                    title : "订单管理"
                },
                component:()=> import("../views/order.vue")
            },
            {
                path: '/rpc',
                name: 'rpc',
                meta: {
                    title: 'rpc接口',
                },
                component: () => import('../views/rpc.vue'),
            },
            {
                path: '/http',
                name: 'http',
                meta: {
                    title: 'http接口',
                },
                component: () => import('../views/http.vue'),
            },
            {
                path: '/log',
                name: 'log',
                meta: {
                    title: '日志',
                },
                component: () => import('../views/log.vue'),
            },
            {
                path: '/user',
                name: 'user',
                meta: {
                    title: '个人中心',
                },
                component: () => import('../views/user.vue'),
            },
            {
                path : "/city",
                name : "city",
                meta : {
                    title: "城市列表"
                },
                component : ()=> import("../views/City.vue")
            },
            {
                path : "/pub_activity",
                name : "pub_activity",
                meta : {
                    title: "发布活动"
                },
                component: ()=> import("../views/activity_pub.vue")
            },
            {
                path: '/activity',
                name: 'activity',
                meta: {
                    title: '活动列表',
                },
                component: () => import('../views/activity.vue'),
            },
            {
                path: '/admin',
                name: 'admin',
                meta: {
                    title: '后台账号',
                },
                component: () => import('../views/admin.vue'),
            }
        ],
    },
    {
        path: '/login',
        name: 'login',
        meta: {
            title: '登录',
        },
        component: () => import('../views/login.vue'),
    },
    {
        path: '/403',
        name: '403',
        meta: {
            title: '没有权限',
        },
        component: () => import('../views/403.vue'),
    },
];

const router = createRouter({
    history: createWebHashHistory(),
    routes,
});

router.beforeEach((to, from, next) => {

    const tokenData = get_token()
    console.log(from.path, "=>", to.path)
    document.title = `${to.meta.title}`;
    if (!tokenData && to.path !== '/login') {
        next('/login');
    } else {
        next()
    }
});

export default router;
