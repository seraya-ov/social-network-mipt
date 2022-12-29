# social-network-mipt

Запуск: 
```
mkdir build
cd build
cmake ..
make
./social_network --login {database login} --password {database password} --database {database} --init_db


http://localhost:8000/user/search
```

Requirements:

```
MySQL-8.0.31
poco-1.12.2
Boost-1.80.0 
mstch-1.0.2
```
