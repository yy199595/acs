return {
    {
        icon = 'Odometer',
        index = 'welcome',
        title = '系统首页',
        admin = false,
    },
    {
        icon = 'PieChart',
        index = '1',
        title = '服务器管理',
        admin = true,
        subs = {
            {
                index = '/dashboard',
                title = '进程信息',
            },
            {
                index = 'rpc',
                title = 'rpc接口',
            },
            {
                index = 'http',
                title = 'http接口',
            },
            {
                index = "mongodb",
                title = "MongoDB管理"
            }
        }
    },
    {
        icon = 'Odometer',
        index = '2',
        title = '用户管理',
        admin = false,
        subs = {
            {
                index = 'account',
                title = '用户列表',
                admin = false,
            },
            {
                index = "role",
                title = "用户信息",
                admin = false
            },
            {
                index = "mch_list",
                title = "商户列表",
                admin = true,
            },
            {
                index = "order",
                title = "订单管理",
                admin = false,
            },
            {
                index = "admin",
                title = "后台账号",
                admin = false,
            }
        }
    },
    {
        icon = "Setting",
        index = '3',
        title = "活动管理",
        admin = false,
        subs = {
            {
                index = "city",
                admin = true,
                title = "城市管理"
            },
            {
                index = "pub_activity",
                title = "发布活动",
                admin = false,
            },
            {
                index = "activity",
                title = "活动列表",
                admin = false,
            }
        }
    }
}