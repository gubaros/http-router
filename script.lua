-- script.lua
math.randomseed(os.time())

request = function()
    local paths = {"/testing", "/asdf", "/google", "/example", "/mach"}
    local path = paths[math.random(1, #paths)]
    return wrk.format("GET", path)
end

