-- 定义节点
local Node = {}
Node.__index = Node

function Node.new(key, value)
    local node = setmetatable({}, Node)
    node.key = key
    node.value = value
    node.prev = nil
    node.next = nil
    return node
end

-- 定义LRUCache类
local LRUCache = {}
LRUCache.__index = LRUCache

function LRUCache.New(capacity)
    local lruCache = setmetatable({}, LRUCache)
    lruCache.capacity = capacity
    lruCache.cache = {}
    lruCache.head = nil
    lruCache.tail = nil
    return lruCache
end

function LRUCache:Get(key)
    local node = self.cache[key]
    if not node then
        return nil
    end

    -- 将访问的节点移动到链表头部
    self:moveToHead(node)

    return node.value
end

function LRUCache:Del(key)
    self.cache[key] = nil
end

function LRUCache:Set(key, value)
    local node = self.cache[key]

    if not node then
        if #self.cache >= self.capacity then
            -- 达到容量上限，淘汰尾部节点
            self:removeTail()
        end

        node = Node.new(key, value)
        self.cache[key] = node

        -- 插入到链表头部
        self:addToHead(node)
    else
        -- 更新节点值，然后移动到链表头部
        node.value = value
        self:moveToHead(node)
    end
end

function LRUCache:addToHead(node)
    if not self.head then
        self.head = node
        self.tail = node
    else
        node.next = self.head
        self.head.prev = node
        self.head = node
    end
end

function LRUCache:removeNode(node)
    if node.prev then
        node.prev.next = node.next
    else
        self.head = node.next
    end

    if node.next then
        node.next.prev = node.prev
    else
        self.tail = node.prev
    end
end

function LRUCache:removeTail()
    if not self.tail then
        return
    end

    local key = self.tail.key
    self.cache[key] = nil
    self:removeNode(self.tail)
end

function LRUCache:moveToHead(node)
    if node == self.head then
        return
    end

    self:removeNode(node)
    self:addToHead(node)
end

return LRUCache