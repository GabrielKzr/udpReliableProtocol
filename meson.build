project(
	'T1 labredes', 
	'cpp', 
 	default_options : ['cpp_std=c++20']
)

incdir = include_directories('src/include')

src = [
	'src/main.cpp', 
    'src/controller/Server.cpp',
	'src/controller/PacketManager.cpp',
	'src/controller/MessageType.cpp',
	'src/controller/Utils.cpp',
	'src/controller/Clock.cpp',
	'src/view/Console.cpp',
]

openssl = dependency('openssl', required: true)

deps = [
	openssl,
]

executable('t1', src, dependencies: deps, include_directories: incdir)