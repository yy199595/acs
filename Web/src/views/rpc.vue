<template>
    <div>
        <el-form>
            <div class="container">
                <el-form inline="inline">
                    <el-form-item label="接口类型">
                        <el-select style='width: 250px' v-model="query.type" placeholder="选择类型">
                            <el-option v-for="item in RpcTypeArray" :key="item.Type" :label="item.Name"
                                       :value="item.Type"/>
                        </el-select>
                    </el-form-item>
                    <el-form-item>
                        <el-form-item label='服务名字'>
                            <el-input v-model='query.name' placeholder="输入服务名字"></el-input>
                        </el-form-item>
                        <el-form-item>
                            <el-button type='primary' @click='onBtnQuery'>查询</el-button>
                        </el-form-item>
                    </el-form-item>
                </el-form>
                <el-table :data="tableData" border class="table" ref="multipleTable"
                          header-cell-class-name="table-header">
                    <el-table-column prop="name" label="名字" align="left"></el-table-column>
                    <el-table-column prop="request" label="请求" align="left"></el-table-column>
                    <el-table-column prop="response" label="返回" align="left"></el-table-column>
                    <el-table-column prop="async" label="协程" align="center"></el-table-column>
                    <el-table-column prop="client" label="客户端接口" align="center"></el-table-column>

                    <el-table-column label="操作" width="220" align="center">
                        <template #default="scope">
                            <el-button text icon="Edit" @click="handleEdit(scope.row)">操作</el-button>
                        </template>
                    </el-table-column>
                </el-table>

                <div class="pagination">
                    <el-pagination
                        background
                        layout="total, prev, pager, next"
                        :current-page="query.page"
                        :page-size="pageSize"
                        :total="pageTotal"
                        @current-change="handlePageChange"
                    ></el-pagination>
                </div>
            </div>
        </el-form>

        <!-- 编辑弹出框 -->
        <el-dialog title="编辑" v-model="editVisible" width="30%">
            <el-form label-width="70px">
                <el-form-item label="名字">
                    <el-input v-model="form.name" :disabled="true"></el-input>
                </el-form-item>

                <el-form-item label="玩家id" v-show="form.client">
                    <el-input v-model="form.user_id" :disabled="false"></el-input>
                </el-form-item>

                <el-form-item label="请求" v-show="form.request.length > 0">
                    <el-input v-model="form.request" :placeholder="form.request" type="textarea" :rows="5"></el-input>
                </el-form-item>

                <el-form-item label="返回" v-show="form.response.length > 0">
                    <el-input v-model="form.response " :placeholder="form.response" type="textarea"
                              :rows="5"></el-input>
                </el-form-item>
            </el-form>
            <template #footer>
				<span class="dialog-footer">
					<el-button type="primary" @click="saveEdit">调用</el-button>
                    <el-button type="primary" @click="saveEdit">禁用</el-button>
                    <el-button @click="editVisible = false">取消</el-button>
				</span>
            </template>
        </el-dialog>
    </div>
</template>

<script setup lang="ts" name="rpc">
import {ref, reactive, onMounted} from 'vue';
import {ElMessage, ElMessageBox} from 'element-plus';
import {Edit, Search, Plus} from '@element-plus/icons-vue';
import {RequestRpcList} from '../api/server';

interface RpcInfo {
    name: string,
    request: string,
    response: string,
    client: boolean,
    open: boolean,
    forward: number,
    async: boolean,
    req: string,
    res: string,
    has_request: boolean,
    has_response: boolean
}

interface RpcInterfaceType {
    Type: number,
    Name: string
}

const RpcTypeArray = ref<RpcInterfaceType[]>([])
RpcTypeArray.value.push({Type: 0, Name: "全部接口"})
RpcTypeArray.value.push({Type: 1, Name: "服务器接口"})
RpcTypeArray.value.push({Type: 2, Name: "客户端接口"})

const query = reactive({
    name: '',
    page: 1,
    type: 1
});

const pageSize = 10
const tableData = ref<RpcInfo[]>([]);
const pageTotal = ref(0);

const onBtnQuery = function () {
    getData()
}

// 获取表格数据
const getData = () => {

    const data = {
        name: query.name,
        page: query.page,
        type: query.type
    }

    RequestRpcList(data).then(res => {
        if (res.data.code == 0) {
            tableData.value = res.data.list
            tableData.value.map((data) => {
                data.has_request = true
                data.has_response = true
                if (!data.request) {
                    data.has_request = false
                    data.request = "无"
                }
                if (!data.response) {
                    data.has_response = false
                    data.response = "无"
                }
            })
            pageTotal.value = res.data.count
            return
        }
        ElMessage.error("获取rpc接口数据失败")
    }).catch(() => {
        ElMessage.error("拉取rpc接口数据失败")
    });
};


// 查询操作
const handleSearch = () => {
    query.page = 1;
    getData();
};
// 分页导航
const handlePageChange = (val: number) => {
    query.page = val;
    getData();
};

// 表格编辑时弹窗和保存
const editVisible = ref(false);
let form = reactive({
    name: '',
    user_id: 0,
    request: '',
    response: '',
    client: false,
});

let idx: number = -1;
const handleEdit = (row: any) => {
    form.name = row.name;
    form.client = row.client
    form.request = row.has_request ? row.req : '';
    form.response = row.has_response ? row.res : '';
    editVisible.value = true
};
const saveEdit = () => {
    editVisible.value = false;
    ElMessage.success(`修改第 ${idx + 1} 行成功`);
    tableData.value[idx].name = form.name;
};

onMounted(() => getData())
</script>

<style scoped>

.table {
    width: 100%;
    font-size: 14px;
}

</style>
