<template>
    <div style="border: 1px solid #ccc">
        <Toolbar
            style="border-bottom: 1px solid #ccc"
            :editor="editorRef"
            :defaultConfig="toolbarConfig"
            :mode="mode"
        />
        <Editor
            style="height: 300px; overflow-y: hidden;"
            v-model="new_text"
            :defaultConfig="editorConfig"
            :mode="mode"
            @onCreated="handleCreated" @change="console.log(new_text)"
        />
    </div>
</template>

<script>
import '@wangeditor/editor/dist/css/style.css' // 引入 css

import {onBeforeUnmount, ref, shallowRef} from 'vue'
import {Editor, Toolbar} from '@wangeditor/editor-for-vue'
import {oss_upload} from "../api/upload"
import {ElMessage} from "element-plus";


export default {
    components: {Editor, Toolbar},
    props: {
        on_text: {
            type: Function,
            required: true
        }
    },
    data() {
        return {
            new_text: this.value
        };
    },
    watch: {
        new_text(newValue) {
            this.new_text = newValue
            if(this.on_text)
            {
                this.on_text(newValue)
            }
        }
    },
    beforeMount() {
        const text = localStorage.getItem("editor_text")
        if(text)
        {
            this.new_text = text;
            console.log(this.new_text)
            localStorage.removeItem("editor_text")
        }
    },
    setup() {
        // 编辑器实例，必须用 shallowRef
        const editorRef = shallowRef()

        // 模拟 ajax 异步获取内容

        const toolbarConfig = {}
        const editorConfig = {placeholder: '请输入内容...'}

        editorConfig.MENU_CONF = {}
        editorConfig.MENU_CONF.uploadImage = {}
        editorConfig.MENU_CONF.uploadImage.customUpload = async function (file, insertImgFn) {
            const url = await oss_upload(file)
            ElMessage.success("上传图片成功")
            insertImgFn(url)
        }


        // 组件销毁时，也及时销毁编辑器
        onBeforeUnmount(() => {
            const editor = editorRef.value
            if (editor == null) return
            editor.destroy()
        })

        const handleCreated = (editor) => {
            editorRef.value = editor // 记录 editor 实例，重要！

        }

        return {
            editorRef,
            mode: 'simple', // 或 'simple'
            toolbarConfig,
            editorConfig,
            handleCreated
        };
    }
}
</script>
