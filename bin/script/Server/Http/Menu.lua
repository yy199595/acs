return {
    {
        icon = 'document',
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
            }
        }
    },
    {
        icon = 'tickets',
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
                index = "admin",
                title = "后台账号",
                admin = false,
            }
        }
    },
    {
        icon = "menu",
        index = '3',
        title = "数据库",
        admin = true,
        subs = {
            {
                index = "mongodb",
                title = "MongoDB"
            },
            {
                index = "mysql",
                title = "MySql"
            },
            {
                index = "pgsql",
                title = "PostgreSQL"
            },
            {
                index = "sqlite",
                title = "Sqlite"
            }
        }
    }
}