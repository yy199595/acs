<template>
  <h3>账号列表</h3>
  <el-form inline="inline">
    <el-form-item label="选择集合" v-if="input.table_list.length > 0" style="width: 100rpx">
      <el-cascader v-model="input.tab" :options="input.table_list" placeholder="选择集合"
                   @change="on_change_collect"></el-cascader>
    </el-form-item>

    <el-form-item label="排序">
      <el-cascader :options="input.sorter" v-model="input.sort" placeholder="排序方式" @change="on_sorter_change"></el-cascader>
    </el-form-item>


    <el-form-item label="条件">
      <el-select @change="on_select_field" style='width: 250px' v-model="input.field" placeholder="选择字段">
        <el-option v-for="item in input.fields" :key="item" :label="item"
                   :value="item"/>
      </el-select>
    </el-form-item>

    <el-form-item label="=">
      <el-input v-model="input.value"></el-input>
    </el-form-item>

    <el-form-item>
      <el-button type="primary" @click="on_btn_query(false)">精确查询</el-button>
      <el-button v-show="input.regex" type="primary" @click="on_btn_query(true)">模糊查询</el-button>
    </el-form-item>

  </el-form>

  <el-dialog v-model="from.show" title="查看集合" destroy-on-close>
    <json-viewer :data="from.data" :editable="from.edit" @update:data="on_input_data"></json-viewer>

    <el-form style="margin-top: 10px" v-show="from.edit">
      <el-form-item>
        <el-button :disabled="from.disable" type="success" @click="on_edit_data()">修改</el-button>
        <el-button @click="from.show = false">取消</el-button>
      </el-form-item>
    </el-form>
  </el-dialog>

  <el-table v-if="input.list.length > 0" :data="input.list" border>
    <el-table-column v-for="key in input.fields" :prop="key" align="center" min-width="120"
                     :label="key" show-overflow-tooltip :formatter="format_text"></el-table-column>
    <el-table-column label="操作" width="240" style="flex-direction: row" fixed="right" align="center">
      <template #default="scope">
        <el-button type="primary" @click="on_btn_look(scope.row, false)">查看</el-button>
        <el-button type="warning" @click="on_btn_look(scope.row, true)">修改</el-button>
        <el-button type="danger" @click="on_btn_delete(scope.row)">删除</el-button>
      </template>
    </el-table-column>
  </el-table>

  <div class="pagination" v-if="input.list.length > 0">
    <el-pagination
        background
        layout="total, prev, pager, next"
        :current-page="input.page"
        :page-size="10"
        :total="input.total"
        @current-change="handlePageChange"
    ></el-pagination>
  </div>
</template>

<script>
import {query_table} from "../api/city";
import {ElMessage, ElMessageBox} from "element-plus";
import {httpRequest} from "../utils/httpRequest"
import JsonViewer from "./json_view.vue";

export default {
  name: "mongodb",
  components: {JsonViewer},
  data() {
    return {
      input : {
        tab : [],
        field : "",
        value : "",
        page : 1,
        total : 0,
        list : [],
        fields : [],
        fields_types : {},
        sorter : [],
        sort : [],
        field_name : "",
        table_list : [],
        regex : false,
      },
      from : {
        data : {},
        line : 0,
        show : false,
        html : "",
        edit : false,
        text : "",
        disable : true,
      }
    }
  },
  methods : {

    on_change_collect() {
      this.input.field = ""
      this.input.value = ""
      this.input.page = 1
      this.on_btn_query()
    },

    on_select_field(res) {
      this.input.regex = this.input.fields_types[res] === "string"
    },

    on_input_data(str) {
      this.from.text = str
      this.from.disable = str === JSON.stringify(this.from.data, null, 2)
    },

    on_sorter_change(res) {
      this.input.page = 1
      this.on_btn_query()
      console.log(JSON.stringify(res))
    },
    async on_edit_data() {
      const tab = this.input.tab.join(".")
      try {
        const updater = JSON.parse(this.from.text)
        if(updater._id !== this.from.data._id) {
          this.from.show = false
          ElMessage.error("不能修改_id字段")
          return
        }
        const response = await httpRequest.POST("/table_mgr/update", {
          tab: tab,
          data: updater
        })
        if (response.data.code === 0) {
          ElMessage.success("修改成功")
        } else {
          ElMessage.error(response.data.error);
        }
        this.from.show = false
      }
      catch (e) {
        ElMessage.error("修改内容不是json")
      }
    },

    on_btn_query(regex) {
      this.input.fields = []

      const data = {
        filter : {},
        tab : this.input.tab.join("."),
        page : this.input.page
      }

      if(this.input.sort.length === 2)
      {
        data.sorter = { }
        const key = this.input.sort[0]
        data.sorter[key] = this.input.sort[1]
      }

      if(this.input.field.length > 0 && this.input.value.length > 0)
      {
        if(regex)
        {
          data.filter[this.input.field] = {
            "$regex" : this.input.value,
            "$options" : "i"
          }
        }
        else
        {
          switch(this.input.fields_types[this.input.field])
          {
            case "number":
            {
              data.filter[this.input.field] = parseInt(this.input.value);
              break
            }
            case "string":
            {
              data.filter[this.input.field] = this.input.value;
              break
            }
            default:
            {
              ElMessage.error("不支持该字段查询")
              return
            }
          }
        }
      }

      query_table(data).then(response=> {

        this.input.list = []
        this.input.total = 0
        if(response.data.code === 0 && Array.isArray(response.data.data.list)) {
          this.input.fields.push("_id")
          if(this.input.sort.length === 2)
          {
            this.input.fields.push(this.input.sort[0])
          }
          if(this.input.field.length > 0) {
            this.input.fields.push(this.input.field)
          }

          this.input.list = response.data.data.list
          this.input.total = response.data.data.count
          for (let i = 0; i < this.input.list.length; i++) {
            const item = this.input.list[i]
            for (const itemKey in item) {
              const value = item[itemKey]
              const index = this.input.fields.findIndex(item => item === itemKey)
              if (index === -1) {
                switch (typeof(value))
                {
                  case "number":
                  {
                    this.input.sorter.push({
                      label: itemKey,
                      value: itemKey,
                      children: [{
                        label: "升序",
                        value: 1
                      }, {
                        label: "降序",
                        value: -1
                      }]
                    })
                    this.input.fields.push(itemKey)
                    break
                  }
                  case "string":
                  {
                    this.input.fields.push(itemKey)
                    break
                  }
                }
                this.input.fields_types[itemKey] = typeof(value)
              }
            }
          }
          const index = this.input.table_list.findIndex(item=> item === this.input.tab)
          if(index === -1)
          {
            this.input.table_list.push(this.input.tab)
            localStorage.setItem("table_list", JSON.stringify(this.input.table_list))
          }
        }
      })
    },
    on_btn_look(data, edit) {
      this.from.text = ""
      this.from.show = true
      this.from.line = 2
      this.from.edit = edit
      this.from.data = data
      this.from.disable = true
    },
    async on_btn_delete(data) {
      await ElMessageBox.confirm('确定删除数据吗？', '提示', {
        type: 'error'
      });
    },

    format_text(row, column, cellValue) {
      const type_name = typeof cellValue
      if(type_name === "number")
      {
        if(column.label.indexOf("time") > -1)
        {
          if(cellValue === 0)
          {
            return cellValue.toString();
          }
          const timestamp = new Date(cellValue * 1000); // 将时间戳转换为Date对象
          return timestamp.toLocaleString();
        }
        else if(column.label.indexOf("amount") > -1)
        {
          return `¥${cellValue / 100.0}`
        }
      }

      if (type_name === "object" || Array.isArray(cellValue)) {
        return JSON.stringify(cellValue)
      } else if (type_name === "boolean") {
        return cellValue ? "true" : "false"
      } else if(type_name === "number") {
        return cellValue
      }
      else if(!cellValue) {
        return "unset"
      }
      return cellValue.toString()
    },
    handlePageChange(page) {
      this.input.page = page
      this.on_btn_query()
    }
  },
  async mounted() {
    const response = await httpRequest.GET("table_mgr/tables")
    if(response.data.code === 0)
    {
      this.input.table_list = response.data.data
    }
    console.log(JSON.stringify(this.input.table_list))
  }
}
</script>

<style scoped>

</style>