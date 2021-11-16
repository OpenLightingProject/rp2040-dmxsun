import React from 'react';

import OverlayTrigger from 'react-bootstrap/OverlayTrigger'
import Tooltip from 'react-bootstrap/Tooltip';

class BoardStatus extends React.Component{

    getToolTipStatusLeds() {
        return (
            <Tooltip><table class="table" style={{color: "#FFFFFF"}}><tbody>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#FF0000', color: '#000000' }}>RED</div></td>
                <td>No module detected in slot</td>
            </tr>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#FFFF00', color: '#000000' }}>YELLOW</div></td>
                <td>Module detected but type unknown</td>
            </tr>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#00FF00', color: '#000000' }}>GREEN</div></td>
                <td>Module detected and type is valid</td>
            </tr>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#0000FF', color: '#FFFFFF' }}>BLUE</div></td>
                <td>Module detected and config loaded from this board</td>
            </tr>
            </tbody></table></Tooltip>
        )
    }

    getToolTipSystemLed() {
        return (
            <Tooltip><table class="table" style={{color: "#FFFFFF"}}><tbody>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#00FF00', color: '#000000' }}>GREEN</div></td>
                <td>Configuration has been loaded from an IO board</td>
            </tr>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#00FFFF', color: '#000000' }}>CYAN</div></td>
                <td>Configuration has been loaded from the base board</td>
            </tr>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#FFFFFF', color: '#000000' }}>WHITE</div></td>
                <td>Fallback configuration active (no other valid config could be found)</td>
            </tr>
            </tbody></table></Tooltip>
        )
    }

    getToolTipUSBLed() {
        return (
            <Tooltip><table class="table" style={{color: "#FFFFFF"}}><tbody>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#00FF00', color: '#000000' }}>GREEN</div></td>
                <td>USB host detected</td>
            </tr>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#0000FF', color: '#FFFFFF' }}>BLINKING<br />BLUE</div></td>
                <td>Traffic on USB connection</td>
            </tr>
            </tbody></table></Tooltip>
        )
    }

    getToolTipWirelessLed() {
        return (
            <Tooltip><table class="table" style={{color: "#FFFFFF"}}><tbody>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#000000', color: '#FFFFFF' }}>OFF</div></td>
                <td>No wireless module detected</td>
            </tr>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#FF0000', color: '#000000' }}>RED</div></td>
                <td>Wireless module idle</td>
            </tr>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#00FF00', color: '#000000' }}>GREEN<br />BLINKING</div></td>
                <td>Wireless transmission activity</td>
            </tr>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#0000FF', color: '#FFFFFF' }}>BLUE<br />BLINKING</div></td>
                <td>Wireless receiving activity</td>
            </tr>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#FF0000', color: '#000000' }}>RED<br />BLINKING</div></td>
                <td>Wireless transmission error (no response)</td>
            </tr>
            </tbody></table></Tooltip>
        )
    }

    getToolTipUniversesLed() {
        return (
            <Tooltip><table class="table" style={{color: "#FFFFFF"}}><tbody>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#000000', color: '#FFFFFF' }}>OFF</div></td>
                <td>All universes are ALL ZERO</td>
            </tr>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#00FF00', color: '#000000' }}>GREEN</div></td>
                <td>Exactly ONE universe has channels in use</td>
            </tr>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#0000FF', color: '#FFFFFF' }}>BLUE</div></td>
                <td>TWO, THREE or FOUR universes have channels in use</td>
            </tr>
            <tr>
                <td><div style={{ border: '1px solid black', padding: '5px', background: '#FFFFFF', color: '#000000' }}>WHITE</div></td>
                <td>More than four universes have channels in use</td>
            </tr>
            </tbody></table></Tooltip>
        )
    }

  render() {
    return(
        <table class="table text-center" style={{ padding: '0px' }}>
            <tbody>
                <tr style={{ padding: '0px' }}>
                    <OverlayTrigger placement="bottom" overlay={ this.getToolTipStatusLeds() }>
                        <td>
                            <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.props.leds[0].static + ' 0%, ' + this.props.leds[0].static + ' 59%, ' + this.props.leds[0].blink + ' 61%, ' + this.props.leds[0].blink + ' 100%)' }}></div><br />
                            Slot 00
                        </td>
                    </OverlayTrigger>
                    <OverlayTrigger placement="bottom" overlay={ this.getToolTipStatusLeds() }>
                        <td>
                            <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.props.leds[1].static + ' 0%, ' + this.props.leds[1].static + ' 59%, ' + this.props.leds[1].blink + ' 61%, ' + this.props.leds[1].blink + ' 100%)' }}></div><br />
                            Slot 01
                        </td>
                    </OverlayTrigger>
                    <OverlayTrigger placement="bottom" overlay={ this.getToolTipStatusLeds() }>
                        <td>
                            <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.props.leds[2].static + ' 0%, ' + this.props.leds[2].static + ' 59%, ' + this.props.leds[2].blink + ' 61%, ' + this.props.leds[2].blink + ' 100%)' }}></div><br />
                            Slot 10
                        </td>
                    </OverlayTrigger>
                    <OverlayTrigger placement="bottom" overlay={ this.getToolTipStatusLeds() }>
                        <td>
                            <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.props.leds[3].static + ' 0%, ' + this.props.leds[3].static + ' 59%, ' + this.props.leds[3].blink + ' 61%, ' + this.props.leds[3].blink + ' 100%)' }}></div><br />
                            Slot 11
                        </td>
                    </OverlayTrigger>
                    <OverlayTrigger placement="bottom" overlay={ this.getToolTipSystemLed() }>
                        <td>
                            <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.props.leds[4].static + ' 0%, ' + this.props.leds[4].static + ' 59%, ' + this.props.leds[4].blink + ' 61%, ' + this.props.leds[4].blink + ' 100%)' }}></div><br />
                            System
                        </td>
                    </OverlayTrigger>
                    <OverlayTrigger placement="bottom" overlay={ this.getToolTipUSBLed() }>
                        <td>
                            <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.props.leds[5].static + ' 0%, ' + this.props.leds[5].static + ' 59%, ' + this.props.leds[5].blink + ' 61%, ' + this.props.leds[5].blink + ' 100%)' }}></div><br />
                            USB
                        </td>
                    </OverlayTrigger>
                    <OverlayTrigger placement="bottom" overlay={ this.getToolTipWirelessLed() }>
                        <td>
                            <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.props.leds[6].static + ' 0%, ' + this.props.leds[6].static + ' 59%, ' + this.props.leds[6].blink + ' 61%, ' + this.props.leds[6].blink + ' 100%)' }}></div><br />
                            Wireless
                        </td>
                    </OverlayTrigger>
                    <OverlayTrigger placement="bottom" overlay={ this.getToolTipUniversesLed() }>
                        <td>
                            <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.props.leds[7].static + ' 0%, ' + this.props.leds[7].static + ' 59%, ' + this.props.leds[7].blink + ' 61%, ' + this.props.leds[7].blink + ' 100%)' }}></div><br />
                            Universes<br />in use
                        </td>
                    </OverlayTrigger>
                </tr>
            </tbody>
        </table>
    );
  }
}

export default BoardStatus;
