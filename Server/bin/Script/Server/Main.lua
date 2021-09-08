Main = {}
function Main.Awake()
    for key, value in pairs(package.loaded) do
        print(key, value)
    end
end

function Main.Start()
    
end
