//STARTER 
//THIS EXERCISE IS TO BE DONE IN A GENERIC C++ COMPILER
//https://www.programiz.com/cpp-programming/online-compiler/
//IF YOU HAVE ISSUES WITH THE COMPILER, CONTACT EMBEDDED LEAD

// super simplified version of how DJIMotor works and how we access the remote data
#include <string>
#include <cstdint>
#include <iostream>
constexpr int CAN_HANDLER_NUMBER = 2;

enum motorType {
    NONE = 0,
    STANDARD = 1,

    GIMBLY = 2,
    GM6020 = 2,

    M3508 = 3,

    C610 = 4,
    M2006 = 4,

    M3508_FLYWHEEL = 5,
};

enum motorDataType {
    ANGLE,
    VELOCITY,
    TORQUE,
    TEMPERATURE,
    POWER,
};

class Remote {
public:

    

enum class SwitchState
{
    UNKNOWN,
    DOWN,
    MID,
    UP
};

    int leftX(){
        return lefX;
    }

    int leftY(){
        return lefY;
    }

    int rightX(){
        return righX;
    }

    int rightY(){
        return righY;
    }

    Remote::SwitchState leftSwitch(){
        return lefS;
    }

    Remote::SwitchState rightSwitch(){
        return righS;
    }

    void read(bool debug = false){
        static int count;
        count %= 660;

        if((count % 15) / 5  == 0)
            lefS = Remote::SwitchState::UP;

        else if((count % 15) / 5 == 1)
            lefS = Remote::SwitchState::MID;

        else if((count % 15) / 5 == 2)
            lefS = Remote::SwitchState::DOWN;

        lefX = count;

        if(debug){
            printf("lX:[%d] lS:[%d]",lefX, (int)lefS);
        }

        count++;
    }

private:
    int lefX, lefY, righX, righY;
    Remote::SwitchState lefS, righS;

};

class CANHandler{
    public:
        enum CANBus {CANBUS_1, CANBUS_2, NOBUS};
};

class DJIMotor{

private:
    static DJIMotor* s_allMotors  [CAN_HANDLER_NUMBER][3][4];
    static bool s_motorsExist     [CAN_HANDLER_NUMBER][3][4];

    short motorID_0;
    short canID_0;
    motorType type;
    CANHandler::CANBus canBus;
    std::string name;

    enum motorMoveMode{
        OFF,
        POS,
        SPD,
        POW,
        ERR
    };

    int value = 0;
    motorMoveMode mode = OFF;

    int16_t motorData[5] = {};

public:
    DJIMotor(short motorID, CANHandler::CANBus canBus, motorType type, const std::string& name) {
        canID_0 = static_cast<short>(motorID - 1);
        motorID_0 = canID_0;
        this -> canBus = canBus;
        this -> type = type;
        this -> name = name;

        if(type == GM6020)
            canID_0 += 4;

        s_allMotors[canBus][canID_0 / 4][canID_0 % 4] = this;
        s_motorsExist[canBus][canID_0 / 4][canID_0 % 4] = true;
    }

    void setPower(int power){
        mode = POW;
        value = power;
    }

    void setSpeed(int speed){
        mode = SPD;
        value = speed;
    }

    void setPosition(int position){
        mode = POS;
        value = position;
    }

    void setOutput(){
        if(mode == POW)
            motorData[POWER] = static_cast<int16_t>(value);

        else if(mode == SPD)
            motorData[VELOCITY] = static_cast<int16_t>(value);

        else if(mode == POS)
            motorData[ANGLE] = static_cast<int16_t>(value);
    }

    static void s_sendValues(){
        for(int canBus = 0; canBus < CAN_HANDLER_NUMBER; canBus ++)
            for(int j = 0; j < 3; j ++)
                for(int k = 0; k < 4; k ++)
                    if(s_motorsExist[canBus][j][k])
                        s_allMotors[canBus][j][k] -> setOutput();

    }

    static void s_getFeedback(){
        // need for PID in real class
    }

    int getData(motorDataType data){
        return motorData[data];
    }
};

DJIMotor* DJIMotor::s_allMotors[2][3][4];
bool DJIMotor::s_motorsExist[2][3][4];
Remote remote;

void remoteRead(bool debug = false){
    remote.read(debug);
}

///////////////
//main.cpp
///////////////


// ONLY ADD CODE BELOW
// Use prints and getData() for debugging purposes
#define JOYSTICK_SENS_YAW 1
#define JOYSTICK_SENS_PITCH 1

#define POS_LOWERBOUND 30
#define POS_UPPERBOUND 150

#define TICKS_PER_REV 8192

int main(){
    
    //SETUP CODE HERE
    DJIMotor yaw_motor(1, CANHandler::CANBUS_2, GIMBLY, "Yaw Motor");
    DJIMotor pitch_motor(2, CANHandler::CANBUS_2, GIMBLY, "Pitch Motor");
    
    int last_yaw_pos = 0, last_pitch_pos = 0;

    while(true){ //main loop
        int yaw_pos = yaw_pos + remote.rightX() * JOYSTICK_SENS_YAW;
        yaw_motor.setPosition(yaw_pos);
        yaw_motor.setOutput();

        /* Assuming that joystick input is the desired relative position to set the motor.
        Also assuming that JOYSTICK_SENS_PITCH is the conversion scaler to degrees.*/
        int pitch_pos = pitch_pos + (remote.rightY() * JOYSTICK_SENS_PITCH);
        if (pitch_pos <= POS_LOWERBOUND) {
            pitch_pos = POS_LOWERBOUND;
        }
        else if (pitch_pos >= POS_UPPERBOUND) {
            pitch_pos = POS_UPPERBOUND;
        }
        pitch_motor.setPosition( (pitch_pos/360) * TICKS_PER_REV);
        pitch_motor.setOutput();
    }
}
