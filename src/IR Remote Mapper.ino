//@Author: Amremad719
/*
    This code is intended for an arduino nano (prbably works on some other models) to be used to
    translate a given IR remote signal to work with some other device.

    In this case I recorded every key on an old IR remote I had and saved it, and I got the
    appropriate IR signals and protocol for the receiver device (in my case an old Sony Hi-Fi system)
    and mapped them in the KEY_MAP table.

    You should be able to change the codes in the KEY_MAP table while keeping it sorted without breaking 
    functionality.

    The general idea is the arduino would listen for given signals, look them up in the KEY_MAP table, and
    transmit the appropriate signal if the recieved signal was found in the KEY_MAP table while handling
    repeating signals i.e the key is pressed down.

    Code is tested and works on the IRremote library version 3.6.0, this isn't the latest version as of the
    time of writing the code but some later versions including the latest (4.2.0) don't decode my remote input
    correctly.

    Notice that some deprecated functions are used, this is because they are simpler to implement and work 
    reliably from my experience so there aren't any reasons to use the later versions.
*/

#include <IRremote.hpp>

//Physcial pins constants
const byte IR_RECEIVE_PIN = 11;
const byte IR_SEND_PIN = 3;

//Transmit protocol releveant constants
const byte PROTOCOL_REPEAT_DELAY = 45;
const byte PROTOCOL_BIT_COUNT = 12;
const byte PROTOCOL_MIN_REPEAT = 3;

//Recieve protocol relevant constants
const unsigned long INPUT_REPEAT_CODE = 0xFFFFFFFF;

//Used to map keys to each other in the KEY_MAP table
struct pair
{
    //The signal to transmit
    unsigned int Target;

    //The received signal
    unsigned long Input;
};

//PRE-SORTED
//All input keys are mapped, some are commented for ease of updating
//Mapping is up to user-prefrence
const pair KEY_MAP[] = {
    {0x11 , 0xFF00FF}, //CD_01 -> 1
    {0xD1 , 0xFF02FD}, //CD_BACK -> PREV
    {0x89 , 0xFF08F7}, //EQ_ARROW_UP -> UP
    {0x841, 0xFF0AF5}, //KEY_TUNER -> LANGUAGE
    {0x111, 0xFF10EF}, //CD_09 -> 9
    // {, 0xFF12ED}, // -> N/P
    {0x766, 0xFF18E7}, //SEL5 -> REPEAT
    {0x211, 0xFF20DF}, //CD_05 -> 5
    {0x1D1, 0xFF22DD}, //CD_STOP -> STOP
    // {, 0xFF278D}, // -> ECHO-
    // {, 0xFF28D7}, // -> TITLE
    {0xA81, 0xFF30CF}, //AMP_POWER -> POWER
    // {, 0xFF32CD}, // -> SLOW
    {0xA41, 0xFF38C7}, //KEY_CD -> ANGLE
    {0x411, 0xFF40BF}, //CD_03 -> 3
    // {, 0xFF42BD}, // -> REV
    {0xE89, 0xFF48B7}, //EQ_ARROW_LEFT -> LEFT
    {0x921, 0xFF4AB5}, //AMP_DIRECT -> PBC
    {0x851, 0xFF50AF}, //CD_11 -> 10+
    {0x8C6, 0xFF52AD}, //DYNAMIC_BASS -> 3D
    {0x9D1, 0xFF58A7}, //CD_PAUSE -> PLAY/PAUSE
    {0x611, 0xFF609F}, //CD_07 -> 7
    {0x426, 0xFF6897}, //SOURROUND_control -> SETUP
    // {, 0xFF708F}, // -> ENTER
    {0x441, 0xFF7887}, //KEY_VIDEO -> OSD
    // {, 0xFF7A85}, // -> B
    {0x811, 0xFF807F}, //CD_02 -> 2
    {0x8D1, 0xFF827D}, //CD_FWD -> NEXT
    {0x889, 0xFF8877}, //EQ_ARROW_DOWN -> DOWN
    // {, 0xFF8A75}, // -> R/L
    {0x7C6, 0xFF906F}, //SOURROUND_mode -> MODE
    // {, 0xFF926D}, // -> PROG
    // {, 0xFF9867}, // -> RETURN
    {0xA11, 0xFFA05F}, //CD_06 -> 6
    // {, 0xFFA25D}, // -> STEP
    // {, 0xFFA857}, // -> MENU
    // {, 0xFFB04F}, // -> OP/CL
    // {, 0xFFB24D}, // -> ECHO+
    // {, 0xFFB847}, // -> MUTE
    {0x3E6, 0xFFBA45}, //EQ -> EQ
    {0xC11, 0xFFC03F}, //CD_04 -> 4
    //{, 0xFFC23D}, // -> FWD
    {0x689, 0xFFC837}, //EQ_ARROW_RIGHT -> RIGHT
    {0x441, 0xFFCA35}, //CD_PLAY -> RESUME
    {0x51 , 0xFFD02F}, //CD_10 -> 10
    {0x481, 0xFFD22D}, //AMP_VOL_UP -> VOL+
    // {, 0xFFD827}, // -> GOTO
    {0xE11, 0xFFE01F}, //CD_08 -> 8
    {0xF66, 0xFFE817}, //P_FILE -> A-B
    {0xD21, 0xFFF00F}, //DISPLAY -> VIEW
    {0xC81, 0xFFF20D}, //AMP_VOL_DOWN -> VOL-
    {0xC41, 0xFFF807}  //KEY_TAPE -> SUBTITLE
    // {, 0xFFFA05} // -> #
};

//Size of array
const byte KEY_MAP_SIZE = sizeof(KEY_MAP) / sizeof(KEY_MAP[0]);

//Handles the data sending
IRsend irsend;

//receives the decoded results
decode_results results;

//Needed for repeating signals i.e the key is pressed down
char LAST_FOUND_CODE_INDEX = -1;
unsigned long LAST_FOUND_CODE_TIME = 0;

void setup() {
    //Initialize IR receiver
    IrReceiver.begin(IR_RECEIVE_PIN);

    //Initialize pinMode
    pinMode(IR_RECEIVE_PIN, INPUT);

    //Initialize IR sender
    irsend.setSendPin(IR_SEND_PIN);
}

/**
* Transmits a given signal using the Sony protocol
* @param code The code to transmit
* @param bits The number of bits of the signal
* @param repeat The number of times to transmit the signal
* @param _delay The delay between repeating signals
*/
void sendSonyRepeat(const unsigned int& code, byte bits = PROTOCOL_BIT_COUNT, byte repeat = PROTOCOL_MIN_REPEAT, byte _delay = PROTOCOL_REPEAT_DELAY)
{
    //retrun if the repeat is a negative number to prevent an infinite loop
    if (repeat < 0) return;

    while (repeat--)
    {
        //Deprecated, please read code header
        irsend.sendSony(code, bits);

        //Delay time between messeges for the protocol
        if (_delay && repeat) {
            delay(_delay);
        }
    }
}

/**
* Looks the given code in the KEY_MAP table Input member variable using binary search
* @param code The code to look for
* @return The index of code in the table, -1 if not found
*/
char findEquivelant(const unsigned long code)
{
    //return if table is empty
    if (!KEY_MAP_SIZE) return -1;

    byte start = 0;
    byte mid;
    byte end = KEY_MAP_SIZE - 1;

    //regular old binary search
    while (start < end)
    {
        mid = (start + end) / 2;

        if (KEY_MAP[mid].Input >= code) {
            end = mid;
        }
        else {
            start = mid + 1;
        }
    }

    //If the index found has the same value we've been looking for return it else return -1 i.e not found
    return (KEY_MAP[end].Input == code ? end : -1);
}

void loop() {
    // put your main code here, to run repeatedly:
    if (IrReceiver.decode(&results))
    {
        IrReceiver.resume();

        //If recieved value is the repeat code and not much time has passed then repeat the last code if it was valid
        if (results.value == INPUT_REPEAT_CODE && millis() - LAST_FOUND_CODE_TIME <= 300 && LAST_FOUND_CODE_INDEX != -1)
        {
            //Make sure the delay between the repeating signals is PROTOCOL_REPEAT_DELAY
            if (millis() - LAST_FOUND_CODE_TIME < PROTOCOL_REPEAT_DELAY) {
                delay(PROTOCOL_REPEAT_DELAY - (millis() - LAST_FOUND_CODE_TIME));
            }

            //Send the signal once 
            sendSonyRepeat(KEY_MAP[LAST_FOUND_CODE_INDEX].Target, PROTOCOL_BIT_COUNT, 1);

            //Store current time to check future repeating signals
            LAST_FOUND_CODE_TIME = millis();
        }
        else
        {
            char RESULTS_INDEX = findEquivelant(results.value);

            //If code found in the table then send the equivelant code
            if (RESULTS_INDEX != -1)
            {
                //Store the index of found code and the time to be used if a repeating signal was found
                LAST_FOUND_CODE_INDEX = RESULTS_INDEX;
                LAST_FOUND_CODE_TIME = millis();

                //Transmit equivilent signal from the KEY_MAP table
                sendSonyRepeat(KEY_MAP[RESULTS_INDEX].Target);
            }
        }
    }
}