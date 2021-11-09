import React from 'react';
import * as Icon from 'react-bootstrap-icons';
import OverlayTrigger from 'react-bootstrap/OverlayTrigger'
import Tooltip from 'react-bootstrap/Tooltip';
import Slider from 'react-input-slider';

class Home extends React.Component {
    constructor() {
        super();
        this.state = {
            config: {},
            ioBoards: {
                "base": {},
                "boards": [
                    {ports: [{connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}]},
                    {ports: [{connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}]},
                    {ports: [{connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}]},
                    {ports: [{connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}, {connector: "", direction: ""}]},
                ],
            },
            inFlight: false,
            leds: [
                {"blink":"#000000","static":"#000000"},
                {"blink":"#000000","static":"#000000"},
                {"blink":"#000000","static":"#000000"},
                {"blink":"#000000","static":"#000000"},
                {"blink":"#000000","static":"#000000"},
                {"blink":"#000000","static":"#000000"},
                {"blink":"#000000","static":"#000000"},
                {"blink":"#000000","static":"#000000"}
            ],
            updateStatusLedsInterval: undefined,
        };
        this.tooltips = {
            statusLeds: "",
        };
        this.setBrightnessTimeout = undefined;
    }


    componentDidMount() {
        this.updateOverview.bind(this)();
        let interval = window.setInterval(this.updateStatuLeds.bind(this), 2000);
        this.setState({
            updateStatusLedsInterval: interval
        });

        // Initialize all tooltips
        var tooltipTriggerList = [].slice.call(document.querySelectorAll('[data-bs-toggle="tooltip"]'))
        var tooltipList = tooltipTriggerList.map(function (tooltipTriggerEl) {
            return new window.bootstrap.Tooltip(tooltipTriggerEl)
        });
    }

    componentWillUnmount() {
        if (this.state.updateStatusLedsInterval) {
            window.clearInterval(this.state.updateStatusLedsInterval);
        }
    }

    updateOverview() {
        // Check if there is already a request running. If so, do nothing
        if (this.state.inFlight) {
            return;
        }

        this.setState({ inFlight: true });
        const url = window.urlPrefix + '/overview/get.json';
        fetch(url)
            .then(res => res.json())
            .catch(
                () => { this.setState({ inFlight: false }); this.updateOverview(); }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('Overview fetched: ', result);
                        this.setState({ inFlight: false, config: result });
                        this.updateIoBoards();
                    }
                }
            ).finally(
                () => {this.setState({ inFlight: false });}
            );
    }

    updateIoBoards() {
        // Check if there is already a request running. If so, do nothing
        if (this.state.inFlight) {
            return;
        }

        this.setState({ inFlight: true });
        const url = window.urlPrefix + '/overview/ioBoards/get.json';
        fetch(url)
            .then(res => res.json())
            .catch(
                () => { this.setState({ inFlight: false }); this.updateIoBoards(); }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('IoBoards fetched: ', result);
                        this.setState({ inFlight: false, ioBoards: result });
                    }
                }
            ).finally(
                () => { this.setState({ inFlight: false }); }
            );
    }

    updateStatuLeds() {
        // Check if there is already a request running. If so, do nothing
        if (this.state.inFlight) {
            return;
        }

        this.setState({ inFlight: true });
        const url = window.urlPrefix + '/overview/statusleds/get.json';
        fetch(url)
            .then(res => res.json())
            .then(
                (result) => {
                    if (result) {
                        console.log('StatusLeds fetched: ', result);
                        this.setState({ inFlight: false, leds: result });
                    }
                }
            ).finally(
                () => { this.setState({ inFlight: false }); }
            );
    }

    setStatusLedBrightness(newValue) {
        // We do the writing behind a "timeout" so we only write
        // once every 100ms in order not to overload the device
        if (this.setBrightnessTimeout) {
            window.clearTimeout(this.setBrightnessTimeout);
        }
        this.setBrightnessTimeout = window.setTimeout(this.realSetStatusLedBrightness.bind(this), 100, newValue);
    }

    realSetStatusLedBrightness(newValue) {
        const url = window.urlPrefix + '/config/statusLeds/brightness/set.json?value=' + newValue;
        fetch(url).finally(() => { this.updateOverview() })
    }

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

    saveConfig(slot, e) {
        console.log('Saving configuration to slot ' +  slot, e.target);

        const url = window.urlPrefix + '/config/save.json?slot=' + slot;
        fetch(url)
            .then(res => res.json())
            .catch(
                () => {
                    e.target.className = "btn btn-danger";
                    window.setTimeout(() => {e.target.className = "btn btn-outline-secondary"}, 3000);
                }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('Overview fetched: ', result);
                        e.target.className = "btn btn-success";
                        window.setTimeout(() => {e.target.className = "btn btn-outline-secondary"}, 3000);
                    }
                }
            );
    }

    enableConfig(slot, e) {
        console.log('enableConfig configuration to slot ', slot);

        const url = window.urlPrefix + '/config/enable.json?slot=' + slot;
        fetch(url)
            .then(res => res.json())
            .catch(
                () => {
                    e.target.className = "btn btn-danger";
                    window.setTimeout(() => {e.target.className = "btn btn-outline-secondary"}, 3000);
                }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('Overview fetched: ', result);
                        e.target.className = "btn btn-success";
                        window.setTimeout(() => {e.target.className = "btn btn-outline-secondary"}, 3000);
                    }
                }
            );
    }

    disableConfig(slot, e) {
        console.log('disableConfig configuration to slot ', slot);

        const url = window.urlPrefix + '/config/disable.json?slot=' + slot;
        fetch(url)
            .then(res => res.json())
            .catch(
                () => {
                    e.target.className = "btn btn-danger";
                    window.setTimeout(() => {e.target.className = "btn btn-outline-secondary"}, 3000);
                }
            )
            .then(
                (result) => {
                    if (result) {
                        console.log('Overview fetched: ', result);
                        e.target.className = "btn btn-success";
                        window.setTimeout(() => {e.target.className = "btn btn-outline-secondary"}, 3000);
                    }
                }
            );
    }

    render() {
        return (
            <div className="console" class="container-fluid">
                <div class="row">
                    <div class="col">
                        &nbsp;
                    </div>
                </div>

                <div class="row">
                    <div class="col">
                        Status Leds (lower half shows blinking aspect):
                        <table class="table text-center" style={{ padding: '0px' }}>
                            <tbody>
                                <tr style={{ padding: '0px' }}>
                                    <OverlayTrigger placement="bottom" overlay={ this.getToolTipStatusLeds() }>
                                        <td>
                                            <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.state.leds[0].static + ' 0%, ' + this.state.leds[0].static + ' 59%, ' + this.state.leds[0].blink + ' 61%, ' + this.state.leds[0].blink + ' 100%)' }}></div><br />
                                            Slot 00
                                        </td>
                                    </OverlayTrigger>
                                    <OverlayTrigger placement="bottom" overlay={ this.getToolTipStatusLeds() }>
                                        <td>
                                            <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.state.leds[1].static + ' 0%, ' + this.state.leds[1].static + ' 59%, ' + this.state.leds[1].blink + ' 61%, ' + this.state.leds[1].blink + ' 100%)' }}></div><br />
                                            Slot 01
                                        </td>
                                    </OverlayTrigger>
                                    <OverlayTrigger placement="bottom" overlay={ this.getToolTipStatusLeds() }>
                                        <td>
                                            <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.state.leds[2].static + ' 0%, ' + this.state.leds[2].static + ' 59%, ' + this.state.leds[2].blink + ' 61%, ' + this.state.leds[2].blink + ' 100%)' }}></div><br />
                                            Slot 10
                                        </td>
                                    </OverlayTrigger>
                                    <OverlayTrigger placement="bottom" overlay={ this.getToolTipStatusLeds() }>
                                        <td>
                                            <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.state.leds[3].static + ' 0%, ' + this.state.leds[3].static + ' 59%, ' + this.state.leds[3].blink + ' 61%, ' + this.state.leds[3].blink + ' 100%)' }}></div><br />
                                            Slot 11
                                        </td>
                                    </OverlayTrigger>
                                    <OverlayTrigger placement="bottom" overlay={ this.getToolTipSystemLed() }>
                                        <td>
                                            <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.state.leds[4].static + ' 0%, ' + this.state.leds[4].static + ' 59%, ' + this.state.leds[4].blink + ' 61%, ' + this.state.leds[4].blink + ' 100%)' }}></div><br />
                                            System
                                        </td>
                                    </OverlayTrigger>
                                    <OverlayTrigger placement="bottom" overlay={ this.getToolTipUSBLed() }>
                                        <td>
                                            <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.state.leds[5].static + ' 0%, ' + this.state.leds[5].static + ' 59%, ' + this.state.leds[5].blink + ' 61%, ' + this.state.leds[5].blink + ' 100%)' }}></div><br />
                                            USB
                                        </td>
                                    </OverlayTrigger>
                                    <OverlayTrigger placement="bottom" overlay={ this.getToolTipWirelessLed() }>
                                        <td>
                                            <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.state.leds[6].static + ' 0%, ' + this.state.leds[6].static + ' 59%, ' + this.state.leds[6].blink + ' 61%, ' + this.state.leds[6].blink + ' 100%)' }}></div><br />
                                            Wireless
                                        </td>
                                    </OverlayTrigger>
                                    <OverlayTrigger placement="bottom" overlay={ this.getToolTipUniversesLed() }>
                                        <td>
                                            <div style={{ border: '1px solid black', borderRadius: '50%', padding: '0px', width: '75px', height: '75px', background: 'linear-gradient(180deg, ' + this.state.leds[7].static + ' 0%, ' + this.state.leds[7].static + ' 59%, ' + this.state.leds[7].blink + ' 61%, ' + this.state.leds[7].blink + ' 100%)' }}></div><br />
                                            Universes<br />in use
                                        </td>
                                    </OverlayTrigger>
                                </tr>
                            </tbody>
                        </table>
                    </div>

                    <div class="col">{ this.state.inFlight && <div class="spinner-border spinner-border-sm" role="status"></div> }</div>

                    <div class="col">
                        &nbsp;
                    </div>
                </div>

                <div class="row">
                    <div class="col">
                        &nbsp;
                    </div>
                </div>

                <div class="row">
                    <div class="col">
                        Running configuration and connected Io boards:
                        <table class="table" style={{ padding: '0px' }}>
                            <tbody>
                                <tr style={{ padding: '0px' }}>
                                    <td colspan="16" style={{ textAlign: 'left', border: '2px solid black' }} >

                                       <table class="table">
                                            <tbody>
                                                <tr>
                                                    <td style={{ fontWeight: 'bold' }}>Board Name:</td>
                                                    <td>{this.state.config.boardName}</td>

                                                    <td style={{ fontWeight: 'bold' }}>Dongle IP address:</td>
                                                    <td>{this.state.config.ownIp}</td>

                                                    <td style={{ fontWeight: 'bold' }}>Serial number:</td>
                                                    <td>{this.state.config.serial}</td>
                                                </tr>
                                                <tr>
                                                    <td style={{ fontWeight: 'bold' }}>Config source:</td>
                                                    <td>{this.state.config.configSource}</td>

                                                    <td style={{ fontWeight: 'bold' }}>IP address assigned to host:</td>
                                                    <td>{this.state.config.hostIp}</td>

                                                    <td style={{ fontWeight: 'bold' }}>Wireless module available:</td>
                                                    <td>{ this.state.config.wirelessModule ? <Icon.CheckSquare width={32} height={32} /> : <Icon.XSquareFill width={32} height={32} /> }</td>
                                                </tr>
                                                <tr>
                                                    <td style={{ fontWeight: 'bold' }}>Firmware version:</td>
                                                    <td>{this.state.config.version}</td>

                                                    <td style={{ fontWeight: 'bold' }}>Network mask:</td>
                                                    <td>{this.state.config.ownMask}</td>

                                                    <td  style={{ fontWeight: 'bold' }}>Status LED brightness:</td>
                                                    <td>
                                                        {this.state.config.statusLedBrightness}
                                                        &nbsp;&nbsp;
                                                        <Slider
                                                            axis="x"
                                                            xstep={10}
                                                            xmin={1}
                                                            xmax={255}
                                                            x={this.state.config.statusLedBrightness}
                                                            onChange={({ x }) => this.setStatusLedBrightness(x)}
                                                        />
                                                    </td>
                                                </tr>
                                                <tr>
                                                    <td colSpan="6" class="text-center">
                                                        <button type="button" class="btn btn-outline-secondary"
                                                          data-bs-toggle="tooltip" data-bs-placement="bottom"
                                                          title="Save configuration to base board"
                                                          onClick={this.saveConfig.bind(this, 4)}>
                                                            <Icon.JournalArrowDown width={32} height={32} pointerEvents="none"/>
                                                        </button>
                                                        &nbsp;
                                                        <button type="button" class="btn btn-outline-secondary"
                                                          data-bs-toggle="tooltip" data-bs-placement="bottom"
                                                          title="Enable the configuration saved on the base board"
                                                          onClick={this.enableConfig.bind(this, 4)}>
                                                              <Icon.JournalCheck width={32} height={32} pointerEvents="none"/>
                                                        </button>
                                                        &nbsp;
                                                        <button type="button" class="btn btn-outline-secondary"
                                                          data-bs-toggle="tooltip" data-bs-placement="bottom"
                                                          title="Disable the configuration saved on the base board"
                                                          onClick={this.disableConfig.bind(this, 4)}>
                                                              <Icon.JournalX width={32} height={32} pointerEvents="none"/>
                                                        </button>
                                                    </td>
                                                </tr>
                                            </tbody>
                                        </table>
                                    </td>
                                </tr>
                                <tr>
                                    {[...Array(4)].map((value, slot) => {
                                        return (
                                            <td colspan="4" style={{ textAlign: 'center', border: '2px solid black' }}>
                                                Slot&nbsp;{slot} { this.state.ioBoards.boards[slot].exist ? <Icon.CheckSquare width={32} height={32} /> : <Icon.XSquareFill width={32} height={32} /> }<br />
                                                Type: { this.state.ioBoards.boards[slot].exist ? this.state.ioBoards.boards[slot].type : '???' }<br />
                                                <button type="button" class="btn btn-outline-secondary"
                                                  data-bs-toggle="tooltip" data-bs-placement="bottom"
                                                  title="Save configuration to this io board"
                                                  onClick={this.saveConfig.bind(this, slot)}>
                                                    <Icon.JournalArrowDown width={32} height={32} pointerEvents="none"/>
                                                </button>
                                                &nbsp;
                                                <button type="button" class="btn btn-outline-secondary"
                                                  data-bs-toggle="tooltip" data-bs-placement="bottom"
                                                  title="Enable the configuration saved on this io board"
                                                  onClick={this.enableConfig.bind(this, slot)}>
                                                      <Icon.JournalCheck width={32} height={32} pointerEvents="none"/>
                                                </button>
                                                &nbsp;
                                                <button type="button" class="btn btn-outline-secondary"
                                                  data-bs-toggle="tooltip" data-bs-placement="bottom"
                                                  title="Disable the configuration saved on this io board"
                                                  onClick={this.disableConfig.bind(this, slot)}>
                                                      <Icon.JournalX width={32} height={32} pointerEvents="none"/>
                                                </button>
                                            </td>
                                        )
                                    })}
                                </tr>

                                <tr>
                                    {[...Array(16)].map((value, index) => {
                                        let slot = Math.floor(index / 4);
                                        let port = index % 4;
                                        return (
                                            <td style={{ textAlign: 'center', border: '2px solid black' }}>
                                                { (
                                                    this.state.ioBoards.boards[slot].exist &&
                                                    this.state.ioBoards.boards[slot].ports[port].direction !== "unknown" &&
                                                    this.state.ioBoards.boards[slot].ports[port].direction !== ""
                                                    ) ?
                                                <div>
                                                { this.state.ioBoards.boards[slot].ports[port].connector === "xlr_5_female" && <img src="media/icon-xlr-5-female.svg" alt="icon-xlr-5-female" width={32} height={32} ></img> }
                                                { this.state.ioBoards.boards[slot].ports[port].connector === "xlr_5_male" && <img src="media/icon-xlr-5-male.svg" alt="icon-xlr-5-male" width={32} height={32}></img> }
                                                { this.state.ioBoards.boards[slot].ports[port].connector === "xlr_3_female" && <img src="media/icon-xlr-3-female.svg" alt="icon-xlr-3-female" width={32} height={32}></img> }
                                                { this.state.ioBoards.boards[slot].ports[port].connector === "xlr_3_male" && <img src="media/icon-xlr-3-male.svg" alt="icon-xlr-3-male" width={32} height={32}></img> }
                                                { this.state.ioBoards.boards[slot].ports[port].connector === "rj45" && <img src="media/icon-rj45.svg" alt="icon-rj45" width={32} height={32}></img> }
                                                { this.state.ioBoards.boards[slot].ports[port].connector === "screws" && <img src="media/icon-screws.svg" alt="icon-screws" width={32} height={32}></img> }
                                                <br />
                                                { this.state.ioBoards.boards[slot].ports[port].direction === "out" && <Icon.ArrowDown width={32} height={32} /> }
                                                { this.state.ioBoards.boards[slot].ports[port].direction === "in" && <Icon.ArrowUp width={32} height={32} /> }
                                                { this.state.ioBoards.boards[slot].ports[port].direction === "switchable" && <Icon.ArrowDownUp width={32} height={32} /> }
                                                </div>
                                                :
                                                <div><Icon.XSquareFill width={32} height={32} /></div>}
                                            </td>
                                        )
                                    })}
                                </tr>
                            </tbody>
                        </table>
                    </div>
                </div>
            </div>
        );
    }
}
export default Home;