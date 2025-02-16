<template>
  <div class="json-viewer">
    <!-- 可编辑模式 -->
    <textarea
        v-if="editable"
        ref="jsonEditor"
        v-model="jsonString"
        @input="handleInput"
        class="json-editor"
        :style="editorStyle"
    ></textarea>

    <!-- 不可编辑模式 -->
    <pre v-else>
            <code ref="jsonContent" class="json" :style="contentStyle"></code>
        </pre>
  </div>
</template>

<script>
export default {
  name: 'JsonViewer',
  props: {
    data: {
      type: [Object, Array],
      required: true
    },
    editable: {
      type: Boolean,
      default: false
    }
  },
  data() {
    return {
      jsonString: JSON.stringify(this.data, null, 2)
    };
  },
  computed: {
    editorStyle() {
      return {
        height: `${this.calculateHeight(this.jsonString)}px`,
        width: `${this.calculateWidth()}px`,
        backgroundColor: '#1c1e22',
        color: '#abb2bf',
        border: 'none',
        padding: '20px',
        fontFamily: 'monospace',
        fontSize: '14px',
        lineHeight: '1.5',
        outline: 'none'
      };
    },
    contentStyle() {
      return {
        height: `${this.calculateHeight(this.jsonString)}px`,
        width: `${this.calculateWidth()}px`
      };
    }
  },
  watch: {
    data: {
      handler(newData) {
        this.jsonString = JSON.stringify(newData, null, 2);
      },
      deep: true
    }
  },
  mounted() {
    if (!this.editable) {
      this.highlightJson();
    }
  },
  methods: {
    highlightJson() {
      this.$refs.jsonContent.textContent = this.jsonString;

      // 动态加载 highlight.js 库和样式
      this.loadHighlightJs(() => {
        window.hljs.highlightElement(this.$refs.jsonContent);
      });
    },
    loadHighlightJs(callback) {
      if (window.hljs) {
        callback();
        return;
      }

      const script = document.createElement('script');
      script.src = 'https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.5.1/highlight.min.js';
      script.onload = () => {
        const jsonScript = document.createElement('script');
        jsonScript.src = 'https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.5.1/languages/json.min.js';
        jsonScript.onload = callback;
        document.head.appendChild(jsonScript);
      };
      document.head.appendChild(script);

      const link = document.createElement('link');
      link.rel = 'stylesheet';
      link.href = 'https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.5.1/styles/atom-one-dark.min.css';
      document.head.appendChild(link);
    },
    calculateWidth() {
      return window.screen.width / 2 - 100
    },

    calculateHeight(jsonString) {
      const lines = jsonString.split('\n').length;
      const lineHeight = 18; // 假设每行高度为 20px
      const maxHeight = window.innerHeight / 2; // 最大高度为屏幕高度的一半
      return Math.min(lines * lineHeight, maxHeight);
    },
    handleInput() {
      try {
        const parsedData = this.jsonString;
        this.$emit('update:data', parsedData); // 将修改后的数据传递给父组件

      } catch (error) {
        console.error('Invalid JSON:', error);
      }
    }
  }
};
</script>

<style scoped>
.json-viewer {
  font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  background-color: #282c34;
  color: #abb2bf;
  display: flex;
  flex-direction: column;
  height: 100%;
  overflow: hidden;
}

pre {
  background-color: #1c1e22;
  color: #f8f8f2;
  padding: 20px;
  overflow: auto;
  margin: 0;
  box-shadow: inset 0 0 10px rgba(0, 0, 0, 0.5);
}

.json-editor {
  background-color: #1c1e22;
  color: #abb2bf;
  border: none;
  padding: 20px;
  font-family: monospace;
  font-size: 14px;
  line-height: 1.5;
  outline: none;
  resize: none;
}
</style>