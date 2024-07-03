<template>
    <div class="avatar" :style="{ width: style_size + 'px', height: style_size + 'px', fontSize:font_size+'px' }">
        {{ getName }}
    </div>
   
</template>

<script>
export default {
    props: ["type", 'userName'],
    data() {
        return{}
    },
    methods:{
        getCharType(char){
            const code = char.charCodeAt(0);
            if ((code >= 65 && code<= 90) || (code >= 97 && code <= 122)) {
                return 'letter';
            } else if ((code >= 19968 && code<= 40959) || (code >= 131072 && code <= 196607)) {
                return 'chinese';
            }
            return 'unknown';
        }
    },
    computed:{
         getName() {
            if(!this.userName){
                return ""
            }
            const len = this.userName.length
            const chars = Array.from(this.userName);
            const end = this.getCharType(chars[len-1])
            if(end == 'chinese'){
                var name = this.userName.substring(len-2,len)
                return name
            }
            return this.userName.substring(0,4)
        },
        style_size(){
            if(this.type == "header"){
                return 45
            }
            else if(this.type == "list"){
                return 40
            }
            else if(this.type == "home"){
                return 80
            }
            return 55
        },
        font_size(){
           if(this.type == "header"){
                return 18
            }
            else if(this.type == "list"){
                return 15
            }
            else if(this.type == "home"){
                return 30
            }
            return 55
        },
    }
};
</script>

<style scoped>
.avatar {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    border-radius: 50%;
    background-color: #00d1b2;
    color: #fff;
    font-weight: bold;
}
</style>
