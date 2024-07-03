<template>
    <div>
        <h3>条款设置</h3>
        <el-form inline="inline">
            <el-form-item>
                <el-select v-model="from.index" @change="on_change">
                    <el-option v-for="item in options"
                               :key="item.code" :label="item.name" :value="item.index"></el-option>
                </el-select>
            </el-form-item>
            <el-form-item>
                <el-button type="primary" @click="from.show=true">修改</el-button>
            </el-form-item>
        </el-form>
        <div>
            <div v-html="from.content" class="rich-text-container" style="overflow : auto"></div>
        </div>
        <el-dialog :title="from.title" v-model="from.show">
            <el-form>
                <el-form-item>
                    <text-editor :on_text="on_text"></text-editor>
                </el-form-item>
                <el-form-item>
                    <el-button type="primary" @click="on_save">保存</el-button>
                </el-form-item>
            </el-form>
        </el-dialog>
    </div>
</template>

<script>

import TextEditor from "./TextEditir.vue"

export default {
    name: "system",
    components : {
        TextEditor
    },
    data() {
        return {
            options : [
                {
                    index : 1,
                    name : "关于我们"
                },
                {
                    index : 2,
                    name : "帮助中心"
                },
                {
                    index : 3,
                    name : "隐私政策"
                },
                {
                    index : 4,
                    name : "联系我们"
                }
            ],
            from : {
                show : false,
                title : "",
                index : 1,
                content : "",
            }
        }
    },
    methods : {
        on_change(index) {
            this.from.index = index
            const idx = this.options.findIndex(item=> item.index === index)
            if(idx !== -1)
            {
                this.from.title = this.options[idx].name
            }
        },
        on_save() {

        },
        on_text(text) {
            this.from.content = text
        }
    },
    mounted() {
        this.on_change(1)
    }
}
</script>

<style scoped>

</style>
