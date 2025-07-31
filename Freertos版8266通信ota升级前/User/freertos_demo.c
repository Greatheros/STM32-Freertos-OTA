#include "freertos_demo.h"
#include <stdio.h>
//Freertos_start()属于初始化程序，负责创建启动程序并开启任务调度器

//物模型参数
const char* servo_positions[] = {"zero", "one", "two", "three", "four"};
const char* led_positions[] = {"false","true"};

QueueHandle_t  LED_Queue,Servo_Queue,Res_Queue;                //队列句柄
QueueHandle_t  Semaphore_Post;                                 //控制上传数据的互斥锁，确保同一时间段只会上传一个数据
SemaphoreHandle_t rev_state;                                   //接收状态信号量
TimerHandle_t  dht11_handle;                                   //软件定时器的句柄

// 全局变量记录上次重连时间
static TickType_t lastReconnect = 0;

//DHT11任务部分
void DHT11_Timer( TimerHandle_t xTimer );

//启动程序部分
void Start_Task(void);
configSTACK_DEPTH_TYPE Start_stack=128;
UBaseType_t Start_Priority=1;
TaskHandle_t Start_handle;

//ONENET_Control程序部分
void ONENET_Control(void * arg);
TaskHandle_t Control_handle;                  //句柄

//LED程序部分
void LED_Task(void * arg);
TaskHandle_t led_handle;                  //句柄

//舵机部分
void Servo_Task(void * arg);
TaskHandle_t servo_handle;                  //句柄

//属性设置响应部分部分
void Attribute_Res(void * arg);
TaskHandle_t res_handle;                  //句柄

//显示程序部分
void Display(void *arg);
TaskHandle_t dis_handle;
StackType_t Display_Stack[dis_depth];
StaticTask_t Display_TCB;
 

/*************************
*函数功能：创建freertos的启动程序，开启任务调度
*输入：无
*输出：无
*************************/
void Freertos_start(void)
{
    //创建队列
    LED_Queue=xQueueCreate(2,sizeof(uint16_t));
    Servo_Queue=xQueueCreate(2,sizeof(uint16_t));
    Res_Queue = xQueueCreate(3,18);
    if(LED_Queue != NULL && Servo_Queue !=NULL)
    {
        printf("设备已全部就绪！\r\n");
    }
    Semaphore_Post = xSemaphoreCreateMutex();   //创建互斥锁
    xSemaphoreGive(Semaphore_Post);            //先进行一次释放
    rev_state=xSemaphoreCreateBinary();        //创建接收状态信号量

    dht11_handle=xTimerCreate("DHT11",pdMS_TO_TICKS(10000), pdTRUE,(void *)1,DHT11_Timer);   //创建定时器，周期为10秒

    //创建启动程序
    xTaskCreate( (TaskFunction_t) Start_Task,
        (const char *) "Start_Task",
        (configSTACK_DEPTH_TYPE) Start_stack,
        (void *)NULL,
        (UBaseType_t) Start_Priority,
        (TaskHandle_t *)&Start_handle
      );
    
    vTaskStartScheduler();  //任务调度器
}


/*************************
*函数功能：freertos的启动程序，负责创建其他任务，自身随后会删除
*输入：无
*输出：无
*************************/
void Start_Task(void)
{
    taskENTER_CRITICAL(); // 进入临界区

    //ONENET控制任务
    xTaskCreate( ONENET_Control, "ONENET_Control", Control_depth,NULL,Control_Priority,&Control_handle);
    //LED运行任务
    xTaskCreate(LED_Task,"LED_Task",led_depth,NULL, led_Priority,&led_handle);
    //Servo运行任务
    xTaskCreate(Servo_Task,"Servo_Task", servo_depth,NULL, servo_Priority,&servo_handle);
    //响应运行任务
    xTaskCreate(Attribute_Res,"XiangYing", res_depth,NULL, res_Priority,&res_handle);
    //显示运行任务
    dis_handle=xTaskCreateStatic(Display,"Display",dis_depth,NULL,dis_Priority,Display_Stack,&Display_TCB);
    //启动软件定时器（DHT11)
    xTimerStart(dht11_handle,0); 


    taskEXIT_CRITICAL(); //退出临界区
    vTaskDelete(NULL); //删除当前任务
}


void Queue_Send(QueueHandle_t Queue,uint16_t status)
{
    if((status) != 0)
    {
        xQueueSend(Queue,&status,0); 
    }
}


/********************************
函数名：LED_Control()
函数功能：平台与LED的交互
输入：无
输出：无
***********************************/
void LED_Control(void)
{
	uint16_t led_status=0;

    for (int i = 0; i < 2; i++) 
    {
        if (1 == MQTT_Get("LightSwitch", led_positions[i])) 
        {
            led_status = i + 1;
            xTaskNotifyGive(res_handle);  
            break;  
        }
    }
	Queue_Send(LED_Queue,led_status);
}
/********************************
函数名：Servo_Control()
函数功能：平台与舵机的交互
输入：无
输出：无
***********************************/
void Servo_Control(void )
{
	uint16_t servo_status=0;

    for (int i = 0; i < 5; i++) 
    {
        if (1 == MQTT_Get("Servoangle", servo_positions[i])) 
        {
            servo_status = i + 1;
            xTaskNotifyGive(res_handle);
            break;  
        }
    }
    
    Queue_Send(Servo_Queue,servo_status);
}

/********************************
函数名：ONENET_Control
函数功能：ONNET平台控制任务
输入：无
输出：无
解释：此任务负责处理平台下发的属性设置请求，并根据请求发送控制LED和舵机的队列消息至相应任务。
***********************************/
void ONENET_Control(void * arg)
{

    if(OTA_Init()!=0)
    {
		W25Q64_SectorErase(0x000000);
        W25Q64_PageProgram(0x000000,(uint8_t *)&Up_pack.ota_key,4);
        W25Q64_PageProgram(0x000100,(uint8_t *)&Up_pack.tid,4);
        W25Q64_PageProgram(0x000200,(uint8_t *)&Up_pack.size,4);
        OLED_Clear();
        OLED_ShowString(0,0,"Upgrade after restart",OLED_6X8);
        OLED_Update();
        delay_ms(2000);
    }
   
    Esp8266_Init();     //ESP8266初始化
    IWDG_Init();        //启动看门狗 
    ESP8266_Clear();
    while(1)
    { 
        if(xSemaphoreTake(rev_state,portMAX_DELAY)==pdTRUE)
        {
            //LED_Tun();
            LED_Control();
            Servo_Control();
        }

    }
}

/********************************
函数名：LED_Task
函数功能：LED灯的运行任务
输入：无
输出：无
解释：在接收到队列 LED_Queue消息后，根据消息内容控制LED，并上传新的属性到服务器
***********************************/
void LED_Task(void * arg)
{
    //char *msg = "LED更新成功";
    while(1)
    {
        uint8_t out=pdFALSE;
        uint8_t led_status = 0;
        if(xQueueReceive(LED_Queue, &led_status, portMAX_DELAY) == pdTRUE)
        {
           	ESP8266_Clear();		
           
            switch(led_status)
            {
                case 1: //LED关闭 
                    LED_off();
                    out=ShangChuan(LightSwitch,"LightSwitch","false");
                    break;
                case 2: //LED打开
                    LED_on();
                    out=ShangChuan(LightSwitch,"LightSwitch","true");
                    break;
                default:
                    break;
            }
            if(out==pdTRUE)xQueueSend(Res_Queue,"LED更新成功",0); 
            else{xQueueSend(Res_Queue,"LED更新失败",0);}     
           
        }
        //vTaskDelay(100); //延时100ms
    }
}

/********************************
函数名：Servo_Task
函数功能：舵机的运行任务
输入：无
输出：无
解释：在接收到队列 Servo_Queue消息后，根据消息内容控制舵机，并上传新的属性到服务器
***********************************/
void Servo_Task(void * arg)
{
    //char *msg = "舵机更新成功";
    
    while(1)
    {
        uint8_t out=pdFALSE;
        uint8_t servo_status = 0;
        if(xQueueReceive(Servo_Queue, &servo_status, portMAX_DELAY) == pdTRUE)
        {					
            ESP8266_Clear();
            switch(servo_status)
            {
                case 1: 
                    Servo_angle(0);
                    out=ShangChuan(Servoangle,"Servoangle","zero");
                    break;
                case 2: 
                    Servo_angle(45);
                    out=ShangChuan(Servoangle,"Servoangle","one");
                    break;
                case 3:
                    Servo_angle(90);
                    out=ShangChuan(Servoangle,"Servoangle","two");
                    break;
                case 4: 
                    Servo_angle(135);
                    out=ShangChuan(Servoangle,"Servoangle","three");
                    break;
                case 5: 
                    Servo_angle(180);
                    out=ShangChuan(Servoangle,"Servoangle","four");
                    break;
                default:
                    break;
            }
            if(out==pdTRUE)xQueueSend(Res_Queue,"舵机更新成功",0); 
            else{xQueueSend(Res_Queue,"舵机更新失败",0);}
            
        }
        //vTaskDelay(100); //延时100ms
    }
}


/**************************
*函数名：Attribute_Res(void * arg)
*函数功能：服务器设置属性后的响应
*输入：无
*输出：无
**************************/
void Attribute_Res(void * arg)
{
    char buffer[156]={0},id[6]={0},Toal_msg[75]={0};
    char msg[32]={0};
   
    while(1)
    {
        uint16_t Num = ulTaskNotifyTake(pdTRUE,portMAX_DELAY);	
		
        while(Num>0)
        {
            if(xQueueReceive(Res_Queue,&msg,0)==pdTRUE)
            {	
                // 检查Toal_msg剩余空间是否足够
                size_t remaining_space = sizeof(Toal_msg) - strlen(Toal_msg) - 1;
                if (strlen(msg) < remaining_space)
                {
                    // 有足够空间，追加消息
                    strcat(Toal_msg, msg);
                    if(Num>1)
                    {   
                        strcat(Toal_msg, " ");  // 用逗号分隔不同消息
                    }
                    memset(msg,0,sizeof(msg));
                }
                else
                {
                    // 空间不足，追加省略标记
                    strcat(Toal_msg, "...");
                    break;  
                }
                Num--;
            }
        }

        if(json_id != NULL)
        { 
            // 从JSON部分解析id
            if(sscanf(json_id, "{\"id\":\"%5[^\"]", id) == 1) 
            {
                // 发送set_reply响应
                taskENTER_CRITICAL(); // 进入临界区
		        snprintf(buffer, sizeof(buffer), 
                "AT+MQTTPUB=0,\"$sys/RUUPvSGLQd/One/thing/property/set_reply\",\"{\\\"id\\\":\\\"%s\\\"\\,\\\"code\\\":200\\,\\\"msg\\\":\\\"%s\\\"}\",0,0",
                id,Toal_msg);									
		        taskEXIT_CRITICAL();
                if(xSemaphoreTake(Semaphore_Post,3000)==pdTRUE)
                {
                    ESP8266_SendCmd(buffer, "OK", 0); //发送响应
                    xSemaphoreGive(Semaphore_Post); //释放信号量	
                }
               
            }
        }
        memset(Toal_msg,0,sizeof(Toal_msg));
        vTaskDelay(20);
    }

}


/********************************
函数名：DHT11_Timer()
函数功能：平台与获取温湿度数据，并实时刷新（以5秒为周期）
输入：无
输出：无
***********************************/
void DHT11_Timer( TimerHandle_t xTimer)
{
    char temp[7] = {0};
    char humi[7] = {0};

    if (Read_DHT11_data() == 1)
    {
        snprintf(temp, sizeof(temp)-1, "%d.%d", TH, TL); // 将温度转换为字符串
        snprintf(humi, sizeof(humi)-1, "%d.%d", RH, RL); // 将湿度转换为字符串
        ShangChuan(doub, "Temp", temp);
        vTaskDelay(3);
        ShangChuan(doub, "Humi", humi);
    }
}

/********************************
函数名：Display()
函数功能：负责在OLED屏上显示当前的温湿度值，0.5秒刷新一次,并兼职喂狗
输入：无
输出：无
***********************************/
void Display(void *arg)
{
    TickType_t xLastWakeTime;
    TaskHandle_t TaskHandle = xTaskGetHandle("Servo_Task");
    while(1)
    {
        IWDG_ReloadCounter();						//重装计数器，喂狗
        //温湿度显示
        OLED_Clear();
        OLED_ShowString(0,0,"Temp:",OLED_8X16);
        OLED_ShowNum(45,0,TH,2,OLED_8X16);
        OLED_ShowChar(72,0,'.',OLED_8X16);
        OLED_ShowNum(81,0,TL,2,OLED_8X16);
    
        OLED_ShowString(0,16,"Humi:",OLED_8X16);
        OLED_ShowNum(45,16,RH,2,OLED_8X16);
        OLED_ShowChar(72,16,'.',OLED_8X16);
        OLED_ShowNum(81,16,RL,2,OLED_8X16);

        OLED_ShowString(63,32,"V1.0",OLED_8X16);
        /*OLED_ShowNum(0,48,uxTaskGetStackHighWaterMark(TaskHandle),3,OLED_8X16);
        OLED_ShowNum(0,32,uxTaskGetStackHighWaterMark(led_handle),3,OLED_8X16);*/
        OLED_Update();
       
        xLastWakeTime = xTaskGetTickCount();
        vTaskDelayUntil(&xLastWakeTime,500);
    }

}


// 空闲任务钩子函数，负责检查网络并重新连接(一分钟一次)
void vApplicationIdleHook(void) {
    static uint8_t initialized = 0;
    uint8_t count=200;
    // 首次调用时初始化
    if(!initialized) {
        lastReconnect = xTaskGetTickCount();
        initialized = 1;
        return;
    }
    
    // 每60秒检查连接
    if(xTaskGetTickCount() - lastReconnect > pdMS_TO_TICKS(60000)) {
        // 检查MQTT连接状态
        if(xSemaphoreTake(Semaphore_Post,portMAX_DELAY)==pdTRUE)
        {
            printf("AT+MQTTCONN?\r\n");
            while(count--)
            {
                    ESP8266_ReadData(Read_Buffer, sizeof(Read_Buffer) - 1);
                    if(strstr((const char *)Read_Buffer, "+MQTTCONN:0,3") != NULL)		//如果检索到关键词
                    {
                        ESP8266_Clear();									//清空缓存
                        if(ESP8266_SendCmd("AT+CWJAP=\""WIFI_NAME"\",\""WIFI_WORD"\"", "OK",1)) 
                        {
                            // 重新订阅主题
                            ESP8266_SendCmd("AT+MQTTSUB=0,\""ONENET_Reply"\",1", "OK", 1);
                            ESP8266_SendCmd("AT+MQTTSUB=0,\""ONENET_Set"\",1", "OK", 1);
                        }
                    }
				ESP8266_Clear();									//清空缓存
                delay_ms(10);
				
            }
			
            lastReconnect = xTaskGetTickCount();
            xSemaphoreGive(Semaphore_Post); //释放信号量	
        }
       
    }

}


/* 为空闲任务定义静态内存 */
static StackType_t xIdleTaskStack[configMINIMAL_STACK_SIZE];
static StaticTask_t xIdleTaskTCB;

/* 为定时器服务任务定义静态内存 */
static StackType_t xTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
static StaticTask_t xTimerTaskTCB;

/* 实现空闲任务的内存回调函数 */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = xIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

/* 实现定时器任务的内存回调函数 */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = xTimerTaskStack;
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    printf("!!! STACK OVERFLOW IN TASK: %s !!!\n", pcTaskName);
    while(1);
}

