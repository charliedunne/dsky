#ifndef _KEYINTMSGDEF_H_
#define _KEYINTMSGDEF_H_

/**
 * @brief KeyModifier Type
 * Different kind of keystrokes, long, short, etc..
 */
typedef enum {
    /**
     * @brief Short press
     */
    KM_SHORT,

    /**
     * @brief Long press
     */
    KM_LONG 
} KeyModifier_e;

/**
 * @brief KeyStroke Struct
 * Struct with the message sent from the KeyInt daemon
 * to the dsky  
 */
typedef struct  {

    /**
     * @brief Id of the key pressed 
     */
    unsigned int keyId;

    /**
     * @brief Modifier of the key pressed 
     */
    KeyModifier_e keyModifier; 

} KeyStroke_t;

/**
 * @brief Key message struct to send in the meessage queue
 */
typedef struct {

    /**
     * @brief Message type, only 1 is used.
     */
  long mType;

  /**
   * @brief Encapsulated data in the message
   */
  KeyStroke_t mData;

} KeyMsg_t;

typedef struct {

    bool enable;
    unsigned int brightness;

} KeyLight_cmd;

typedef union {
    
    KeyLight_cmd keylight;

} Command_t;

/**
 * @brief Struct for the input command messages
 * 
 */
typedef struct {

    /**
     * @brief Message type, only 1 is used.
     */
    long mType;

    /**
     * @brief Encapsulated data in the message.
     * @attention Note that it contains a union of the different
     * kind of command messages that can be received.
     */
    Command_t cmd;

} CommMsg_t;

#endif /* _KEYINTMSGDEF_H_ */
