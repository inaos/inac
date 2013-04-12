--[[sections = {}
sections.Debug = {
  name = "Debug",
  named = false,
  required = true,
  keys = {
    command_latency = {
      required = true,
      typename = "number"
    },
    other_latency = {
      required = false,
      typename = "number"
    }
  },
  configured = false
}
sections.Iface = {
  name = "Iface",
  named = true,
  required = true,
  keys = {
    ip = {
      required = true,
      typename = "string"
    },
    mask = {
      required = true,
      typename = "string"
    },
  },
  configured = false
}
]]

-- Example config
--example =
Debug {
	command_latency=1000
}
Iface "lo0" { 
	ip="127.0.0.1", 
	mask="255.0.0.0" 
}
