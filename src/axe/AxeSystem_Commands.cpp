#include "AxeSystem.h"

void AxeSystem::requestFirmwareVersion() {
	sendSysEx(6, (byte*) REQUEST_FIRMWARE_VERSION_COMMAND_6_BYTES);
}

void AxeSystem::requestPresetName(int preset) {
  if (preset == -1) {
    sendSysEx(8, (byte*) REQUEST_PRESET_NAME_COMMAND_8_BYTES);
  } else {
    byte command[8];
    byte bank = preset / BANK_SIZE;
    memcpy(command, REQUEST_PRESET_NAME_COMMAND_8_BYTES, 8);
    command[5] = preset - (bank * BANK_SIZE);
    command[6] = bank;
    sendSysEx(8, (byte*) command);
  }
}

void AxeSystem::requestSceneName() {
	sendSysEx(7, (byte*) REQUEST_SCENE_NAME_COMMAND_7_BYTES);
}

void AxeSystem::requestSceneNumber() {
	sendSysEx(7, (byte*) REQUEST_SCENE_NUMBER_COMMAND_7_BYTES);
}

void AxeSystem::requestEffectDetails() {
	sendSysEx(6, (byte*) REQUEST_EFFECT_DUMP_6_BYTES);
}

void AxeSystem::sendSceneChange(byte scene) {
	byte command[7];
	memcpy(command, REQUEST_SCENE_NUMBER_COMMAND_7_BYTES, 7);
	command[5] = scene - 1;
	sendSysEx(7, (byte*) command);
}

void AxeSystem::requestTempo() {
	sendSysEx(8, (byte*) REQUEST_TEMPO_COMMAND_8_BYTES);
}

void AxeSystem::setTempo(Tempo tempo) {
  byte command[8];
	memcpy(command, REQUEST_TEMPO_COMMAND_8_BYTES, 8);
	byteToMidiBytes(tempo, &command[5], &command[6]);
	command[5] = tempo % BANK_SIZE;
  command[6] = tempo / BANK_SIZE;
	sendSysEx(8, (byte*) command);

	//Axe won't notify us of this, so send a manual system change event
	_tempo = tempo;
	callSystemChangeCallback();
}

void AxeSystem::sendTap() {
	sendSysEx(6, (byte*) TAP_TEMPO_PULSE_COMMAND_6_BYTES);
}

void AxeSystem::toggleTuner() {
	_tunerEngaged ? disableTuner() : enableTuner();
}

void AxeSystem::enableTuner() {
	_tunerEngaged = true;
  sendSysEx(7, (byte*) ENABLE_TUNER_COMMAND_7_BYTES);
	callTunerStatusCallback(_tunerEngaged);
}

void AxeSystem::disableTuner() {
	_tunerEngaged = false;
  sendSysEx(7, (byte*) DISABLE_TUNER_COMMAND_7_BYTES);
	callTunerStatusCallback(_tunerEngaged);
}

void AxeSystem::sendPresetIncrement() {
  int number = _preset.getPresetNumber();
  if (isValidPresetNumber(number)) {
    if (++number > (BANK_SIZE * MAX_BANKS) - 1) {
      number = 0;
    }
    sendPresetChange(number);
  }
}

void AxeSystem::sendPresetDecrement() {
  int number = _preset.getPresetNumber();
  if (isValidPresetNumber(number)) {
    if (number == 0) {
      number = (BANK_SIZE * MAX_BANKS) - 1;
    } else {
      number--;
    }
    sendPresetChange(number);
  }
}

void AxeSystem::sendSceneIncrement() {
  byte number = _preset.getSceneNumber();
  if (isValidSceneNumber(number)) {
    if (++number > MAX_SCENES) {
      number = 0;
    }
    sendSceneChange(number);
  }
}

void AxeSystem::sendSceneDecrement() {
  byte number = _preset.getSceneNumber();
  if (isValidSceneNumber(number)) {
    if (number == 0) {
      number = MAX_SCENES - 1;
    } else {
      number--;
    }
    sendSceneChange(number);
  }
}
