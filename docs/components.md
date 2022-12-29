# Компонентная архитектура
<!-- Состав и взаимосвязи компонентов системы между собой и внешними системами с указанием протоколов, ключевые технологии, используемые для реализации компонентов.
Диаграмма контейнеров C4 и текстовое описание. 
-->
## Компонентная диаграмма

```plantuml
@startuml
!include https://raw.githubusercontent.com/plantuml-stdlib/C4-PlantUML/master/C4_Container.puml

AddElementTag("microService", $shape=EightSidedShape(), $bgColor="CornflowerBlue", $fontColor="white", $legendText="microservice")
AddElementTag("storage", $shape=RoundedBoxShape(), $bgColor="lightSkyBlue", $fontColor="white")

Person(admin, "Администратор")
Person(user, "Пользователь")

System_Boundary(social_network, "Социальная сеть") {
   Container(app, "Клиентское веб-приложение", "JavaScript, HTML, CSS, React", "Клиентское веб-приложение")
   Container(client_service, "Сервис авторизации", "C++", "Сервис управления пользователями", $tags = "microService")    
   Container(feed_service, "Сервис стена", "C++", "Сервис управления стенами пользователей", $tags = "microService") 
   Container(message_service, "Сервис сообщений", "C++", "Сервис управления сообщениями пользователя", $tags = "microService")       
   ContainerDb(db, "База данных", "MySQL", "Хранение данных пользователя", $tags = "storage")
   
}

Rel_D(admin, app, "Добавление/просмотр информации о пользователях")
Rel_D(user, app, "Регистрация, просмотр стены, диалогов и стен других пользователей, написание сообщений")

Rel_D(app, client_service, "", "Работа с пользователями", "http://localhost/user")
Rel_D(client_service, db, "", "INSERT/SELECT/UPDATE", "SQL")

Rel_D(app,feed_service,"","Работа со стеной пользователя","http://localhost/<email>/feed")
Rel_D(feed_service,db,"","INTSERT/SELECT/UPDATE","SQL")
Rel_D(app,message_service,"","Работа с сообщениями пользователя","http://localhost/<email>/messages")
Rel_D(message_service,db,"", "INTSERT/SELECT/UPDATE","SQL")


@enduml
```
## Список компонентов
### Сервис авторизации
**API**:
-	Создание нового пользователя
     - Входыне параметры: Имя, Фамилия, email
     - Выходные параметры, отсутствуют
-	Поиск пользователя по логину
     - Входные параметры: email
     - Выходные параметры: Имя, Фамилия, email
-	Поиск пользователя по маске имя и фамилии
     - Входные параметры: маска фамилии, маска имени
     - Выходные параметры: массив кортежей - Имя, Фамилия, email

### Сервис стена
**API**:
- Добавление записи на стену
  - Входные параметры: user_id, Дата создания записи, Текст записи
  - Выходыне параметры: отсутствуют
- Загрузка стены пользователя
  - Входные параметры: user_id
  - Выходные параметры: массив пар время - текст записи


### Сервис сообщений
**API**:
- Отправка сообщения пользователю
  - Входные параметры: user_id, Время отправки, Текст сообщения
  - Выходные параметры: отсутствуют
- Получение списка сообщения для пользователя
  - Входные параметры: user_id
  - Выходные параметры: массив пар время - сообщение
