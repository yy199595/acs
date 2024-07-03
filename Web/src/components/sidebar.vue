<template>
    <div class="sidebar">
        <el-menu class="sidebar-el-menu" :default-active="onRoutes" :collapse="sidebar.collapse"
                 background-color="#324157" text-color="#bfcbd9" active-text-color="#20a0ff"
                 unique-opened router>
            <template v-for="item in from_data">
                <template v-if="item.subs">
                    <el-sub-menu :index="item.index" :key="item.index" v-if="item.show">
                        <template #title>
                            <el-icon>
                                <component :is="item.icon"></component>
                            </el-icon>
                            <span>{{ item.title }}</span>
                        </template>
                        <template v-for="subItem in item.subs">
                            <div v-if="subItem.show">
                                <el-sub-menu
                                    v-if="subItem.subs"
                                    :index="subItem.index"
                                    :key="subItem.index"
                                >
                                    <template #title>{{ subItem.title }}</template>
                                    <el-menu-item v-for="(threeItem, i) in subItem.subs" :key="i" :index="threeItem.index">
                                        {{ threeItem.title }}
                                    </el-menu-item>
                                </el-sub-menu>
                                <el-menu-item v-else :index="subItem.index">
                                    {{ subItem.title }}
                                </el-menu-item>
                            </div>

                        </template>
                    </el-sub-menu>
                </template>
                <template v-else>
                    <el-menu-item :index="item.index" :key="item.index">
                        <el-icon>
                            <component :is="item.icon"></component>
                        </el-icon>
                        <template #title>{{ item.title }}</template>
                    </el-menu-item>
                </template>
            </template>
        </el-menu>
    </div>
</template>

<script setup lang="ts">
import {computed, onMounted, ref} from 'vue';
import {is_admin} from "../api/token";
import {useSidebarStore} from '../store/sidebar';
import {useRoute, useRouter} from 'vue-router';
import {RequestGetMenus} from "../api/server"
import {ElMessage} from 'element-plus';

const route = useRoute();
const onRoutes = computed(() => {
    return route.path;
});
interface MenuSubs {
    index : string,
    title : string,
    show : boolean,
    admin : boolean,
    subs : MenuSubs[]
}

interface MenuInfo {
    icon : string,
    index : string,
    title : string,
    show : boolean,
    admin : boolean,
    subs : MenuSubs[]
}

const from_data = ref<MenuInfo[]>([])


onMounted(async () => {
    const router = useRouter();
    try {
        const response = await RequestGetMenus()
        const result = response.data
        if (result.code != 0) {
            await router.push("/login")
            ElMessage.error("获取菜单失败，请检查网络")
            return
        }
        from_data.value = result.data as MenuInfo[]
        for (const resultElement of from_data.value) {
            resultElement.show = true
            if(resultElement.admin && !is_admin())
            {
                resultElement.show = false
            }
            if(Array.isArray(resultElement.subs))
            {
                for (let i = 0; i < resultElement.subs.length; i++) {
                    const resultElementElement = resultElement.subs[i]
                    resultElementElement.show = true
                    if(resultElementElement.admin && !is_admin())
                    {
                        resultElementElement.show = false
                    }
                }
            }
        }
    } catch (error) {

        await router.push("/login")
        console.error(error)
        ElMessage.error("获取菜单失败，请检查网络")
    }
})

const sidebar = useSidebarStore();
</script>

<style scoped>
.sidebar {
    display: block;
    position: absolute;
    left: 0;
    top: 70px;
    bottom: 0;
    overflow-y: scroll;
}

.sidebar::-webkit-scrollbar {
    width: 0;
}

.sidebar-el-menu:not(.el-menu--collapse) {
    width: 250px;
}

.sidebar > ul {
    height: 100%;
}
</style>
