
if not params then
  print("params are nil")
end

for i,v in pairs(params) do
  if tonumber(v) then
    print(i .. " " .. v)
  end
end

