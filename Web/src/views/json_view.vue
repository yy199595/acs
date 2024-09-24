<template>
    <div class="json-viewer">
        <pre><code ref="jsonContent" class="json"></code></pre>
    </div>
</template>

<script>
export default {
    name: 'JsonViewer',
    props: {
        data: {
            type: Object,
            required: true
        }
    },
    mounted() {
        this.highlightJson();
    },
    methods: {
        highlightJson() {
            this.$refs.jsonContent.textContent = JSON.stringify(this.data, null, 2);

            // 动态加载 highlight.js 库和样式
            this.loadHighlightJs(() => {
                window.hljs.highlightElement(this.$refs.jsonContent);
                this.setDynamicHeight();
            });
        },
        loadHighlightJs(callback) {
            // 检查是否已经加载了 highlight.js
            if (window.hljs) {
                callback();
                return;
            }

            const script = document.createElement('script');
            script.src = 'https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.5.1/highlight.min.js';
            script.onload = () => {
                const jsonScript = document.createElement('script');
                jsonScript.src = 'https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.5.1/languages/json.min.js';
                jsonScript.onload = () => {
                    callback();
                };
                document.head.appendChild(jsonScript);
            };
            document.head.appendChild(script);

            // 动态加载样式
            const link = document.createElement('link');
            link.rel = 'stylesheet';
            link.href = 'https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.5.1/styles/atom-one-dark.min.css';
            document.head.appendChild(link);
        },
        setDynamicHeight() {
            // 计算JSON内容的行数，并设置适当的高度
            const lineHeight = 15; // 假设每行的高度为24px
            const lines = this.$refs.jsonContent.textContent.split('\n').length;
            const maxHeight = Math.min((lines * lineHeight), 600)
            this.$refs.jsonContent.style.height = `${maxHeight}px`;
            console.log(this.$refs.jsonContent.style.height)
        }
    }
};
</script>

<style scoped>
.json-viewer {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    margin: 0;
    padding: 0;
    background-color: #282c34;
    color: #abb2bf;
    display: flex;
    flex-direction: column;
    height: 100%;
    overflow: hidden;
}
h1 {
    color: #61dafb;
    text-align: center;
    margin: 10px 0;
}
pre {
    background-color: #1c1e22;
    color: #f8f8f2;
    padding: 20px;
    overflow: auto;
    margin: 0;
    box-shadow: inset 0 0 10px rgba(0, 0, 0, 0.5);
}
</style>
