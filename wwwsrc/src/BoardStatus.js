import React from 'react';

import Slider from 'react-input-slider';
import * as Icon from 'react-bootstrap-icons';

// Direct import required since we use the constructor
import Tooltip from 'bootstrap/js/dist/tooltip'

class BoardStatus extends React.Component{
    constructor() {
        super();

        this.setBrightnessTimeout = undefined;

        this.modalBoardName = this.modalBoardName.bind(this);
        this.modalOwnIp = this.modalOwnIp.bind(this);
        this.modalWireless = this.modalWireless.bind(this);
    }

    componentDidMount() {
        // Initialize all tooltips
        var tooltipTriggerList = [].slice.call(document.querySelectorAll('[data-bs-toggle="tooltip"]'))
        tooltipTriggerList.map(function (tooltipTriggerEl) {
            return new Tooltip(tooltipTriggerEl)
        });

        // Focus the modals' input fields when they open
        var myModal;

        myModal = document.getElementById('modalBoardName')
        if (myModal) {
            document.getElementById('modalBoardName').configured = false;
            myModal.addEventListener('shown.bs.modal', () => {
                if (!document.getElementById('modalBoardName').configured) {
                    document.getElementById('modalBoardNameInput').value = this.props.config.boardName;
                    document.getElementById('modalBoardNameInput').focus();
                    document.getElementById('modalBoardName').configured = true;
                }
            });
        }

        myModal = document.getElementById('modalOwnIp')
        if (myModal) {
            document.getElementById('modalOwnIp').configured = false;
            myModal.addEventListener('shown.bs.modal', () => {
                if (!document.getElementById('modalOwnIp').configured) {
                    document.getElementById('modalOwnIpInput').value = this.props.config.ownIp;
                    document.getElementById('modalOwnIpInput').focus();
                    document.getElementById('modalOwnIp').configured = true;
                }
            });
        }

        myModal = document.getElementById('modalWireless')
        if (myModal) {
            document.getElementById('modalWireless').configured = false;
            myModal.addEventListener('shown.bs.modal', () => {
                let modalName = 'modalWireless';
                console.log('shown.bs.modal: modalWireless. Configured: ' + document.getElementById('modalWireless').configured);
                if (!document.getElementById('modalWireless').configured) {
                    document.getElementById(modalName + 'InputRole').value = this.props.wireless.role;
                    document.getElementById(modalName + 'InputChannel').value = this.props.wireless.channel;
                    document.getElementById(modalName + 'InputAddress').value = this.props.wireless.address;
                    document.getElementById(modalName + 'InputCompress').checked = this.props.wireless.compress;
                    document.getElementById(modalName + 'InputSparse').checked = this.props.wireless.sparse;
                    document.getElementById(modalName + 'InputRate').value = this.props.wireless.dataRate;
                    document.getElementById(modalName + 'InputPower').value = this.props.wireless.txPower;
                    document.getElementById(modalName).configured = true;
                }
            });
        }

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
        fetch(url).finally(() => { this.props.updateOverview() })
    }

    loadConfig(slot, e) {
        console.log('Loading configuration from slot ' +  slot, e.target);

        const url = window.urlPrefix + '/config/load.json?slot=' + slot;
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
            )
            .finally(() => { this.props.updateOverview() })
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

    handleModalInput(e) {
        let modalName = e.currentTarget.parentNode.parentNode.parentNode.parentNode.id;
        let paramName = modalName.substring(5);

        if (paramName === 'Wireless') {
            let url = window.urlPrefix + '/config/wireless/set.json?';

            url += 'role=' + encodeURIComponent(document.getElementById(modalName + 'InputRole').value) + '&';
            url += 'channel=' + encodeURIComponent(document.getElementById(modalName + 'InputChannel').value) + '&';
            url += 'address=' + encodeURIComponent(document.getElementById(modalName + 'InputAddress').value) + '&';
            url += 'compress=' + encodeURIComponent(document.getElementById(modalName + 'InputCompress').checked) + '&';
            url += 'sparse=' + encodeURIComponent(document.getElementById(modalName + 'InputSparse').checked) + '&';
            url += 'rate=' + encodeURIComponent(document.getElementById(modalName + 'InputRate').value) + '&';
            url += 'power=' + encodeURIComponent(document.getElementById(modalName + 'InputPower').value) + '&';

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
        } else {
            let newValue = document.getElementById(modalName + 'Input').value;

            const url = window.urlPrefix + '/config/set.json?' + paramName + '=' + encodeURIComponent(newValue);
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
                )
                .finally(() => { this.props.updateOverview() });
        }
    }

    modalBoardName() {
        // The modals used to have the className "fade" as well. However, this
        // broke the "static" backdrop and the modals closed when clicked, even
        // if they shouldn't. Maybe a bootstrap-bug?
        return(
            <div className="modal" id="modalBoardName" data-bs-backdrop="static" data-bs-keyboard="false" tabIndex="-1" aria-labelledby="modalBoardNameLabel" aria-hidden="true">
                <div className="modal-dialog">
                    <div className="modal-content">
                        <div className="modal-header">
                            <h5 className="modal-title" id="modalBoardNameLabel">Edit board name</h5>
                            <button type="button" className="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                        </div>
                        <div className="modal-body">
                            <input type="text" className="form-control" id="modalBoardNameInput" />
                        </div>
                        <div className="modal-footer">
                            <button type="button" className="btn btn-secondary" data-bs-dismiss="modal">Close</button>
                            <button type="button" className="btn btn-primary" onClick={this.handleModalInput.bind(this)}>Save changes</button>
                        </div>
                    </div>
                </div>
            </div>
        )
    }

    modalOwnIp() {
        // The modals used to have the className "fade" as well. However, this
        // broke the "static" backdrop and the modals closed when clicked, even
        // if they shouldn't. Maybe a bootstrap-bug?
        return(
            <div className="modal" id="modalOwnIp" data-bs-backdrop="static" data-bs-keyboard="false" tabIndex="-1" aria-labelledby="modalOwnIpLabel" aria-hidden="true">
                <div className="modal-dialog">
                    <div className="modal-content">
                        <div className="modal-header">
                            <h5 className="modal-title" id="modalOwnIpLabel">Edit board's IP address</h5>
                            <button type="button" className="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                        </div>
                        <div className="modal-body">
                            <input type="text" className="form-control" id="modalOwnIpInput" />
                        </div>
                        <div className="modal-footer">
                            <button type="button" className="btn btn-secondary" data-bs-dismiss="modal">Close</button>
                            <button type="button" className="btn btn-primary" onClick={this.handleModalInput.bind(this)}>Save changes</button>
                        </div>
                    </div>
                </div>
            </div>
        )
    }

    modalWireless() {
        // The modals used to have the className "fade" as well. However, this
        // broke the "static" backdrop and the modals closed when clicked, even
        // if they shouldn't. Maybe a bootstrap-bug?
        return(
            <div className="modal" id="modalWireless" data-bs-backdrop="static" data-bs-keyboard="false" tabIndex="-1" aria-labelledby="modalWirelessLabel" aria-hidden="true">
                <div className="modal-dialog">
                    <div className="modal-content">
                        <div className="modal-header">
                            <h5 className="modal-title" id="modalWirelessLabel">Edit board's wireless config</h5>
                            <button type="button" className="btn-close" data-bs-dismiss="modal" aria-label="Close"></button>
                        </div>
                        <div className="modal-body">

                            <div className="form-floating">
                                <select className="form-select" aria-label="Radio role" id="modalWirelessInputRole" defaultValue="0">
                                   {/* TODO: Remove fixed values here, get them from the ENUM in the firmware */}
                                   <option value="0">Sniffer (listen only)</option>
                                   <option value="1">Broadcast (All-To-All)</option>
                                   <option value="2">Mesh</option>
                                </select>
                                <label htmlFor="modalWirelessInputRole" className="form-label">Radio role:</label>
                            </div>
                            <br />

                            <div className="form-floating">
                                <input type="number" min="0" max="127" className="form-control" id="modalWirelessInputChannel" placeholder="0" defaultValue="0" />
                                <label htmlFor="modalWirelessInputChannel" className="form-label">Radio channel:</label>
                            </div>
                            <br />

                            <div className="form-floating">
                                <input type="number" min="0" max="255" className="form-control" id="modalWirelessInputAddress" placeholder="0" defaultValue="0" />
                                <label htmlFor="modalWirelessInputAddress" className="form-label">Radio address:</label>
                            </div>
                            <br />

                            <div className="form-check form-switch">
                                <input className="form-check-input" type="checkbox" id="modalWirelessInputCompress" />
                                <label className="form-check-label" htmlFor="modalWirelessInputCompress">Compress data</label>
                            </div>
                            <br />

                            <div className="form-check form-switch">
                                <input className="form-check-input" type="checkbox" id="modalWirelessInputSparse" />
                                <label className="form-check-label" htmlFor="modalWirelessInputSparse">Allow sparse sends</label>
                            </div>
                            <br />

                            <div className="form-floating">
                                <select className="form-select" aria-label="Radio role" id="modalWirelessInputRate" defaultValue="0">
                                   {/* TODO: Remove fixed values here, get them from the ENUM in the firmware */}
                                   <option value="0">1 Mbps</option>
                                   <option value="1">2 Mbps</option>
                                   <option value="2">250 kbps</option>
                                </select>
                                <label htmlFor="modalWirelessInputRate" className="form-label">Radio data rate:</label>
                            </div>
                            <br />

                            <div className="form-floating">
                                <select className="form-select" aria-label="Radio role" id="modalWirelessInputPower" defaultValue="0">
                                   {/* TODO: Remove fixed values here, get them from the ENUM in the firmware */}
                                   <option value="0">min (LNA: ~-6 dBm, no LNA: ~-18 dBm)</option>
                                   <option value="1">low (LNA: ~0 dBm, no LNA: ~-12 dBm)</option>
                                   <option value="2">high (LNA: ~+3 dBm, no LNA: ~-6 dBm)</option>
                                   <option value="3">max (LNA: ~+7 dBm, no LNA: ~0 dBm)</option>
                                </select>
                                <label htmlFor="modalWirelessInputPower" className="form-label">Radio TX power:</label>
                            </div>

                        </div>
                        <div className="modal-footer">
                            <button type="button" className="btn btn-secondary" data-bs-dismiss="modal">Close</button>
                            <button type="button" className="btn btn-primary" onClick={this.handleModalInput.bind(this)}>Save changes</button>
                        </div>
                    </div>
                </div>
            </div>
        )
    }

  render() {
    return(
        <table className="table" style={{ padding: '0px' }}>
            <tbody>
                <tr style={{ padding: '0px' }}>
                    <td colSpan="16" style={{ textAlign: 'left', border: '2px solid black' }} >

                       <table className="table">
                            <tbody>
                                <tr>
                                    <td style={{ fontWeight: 'bold' }}>Board Name:</td>
                                    <td>
                                        { this.props.withEdit &&
                                        <span data-bs-toggle="modal" data-bs-target="#modalBoardName">
                                            <this.modalBoardName />
                                            <button type="button" className="btn btn-outline-secondary p-1 m-1"
                                            data-bs-toggle="tooltip" data-bs-placement="bottom"
                                            title="Edit">
                                                <Icon.Pencil width={24} height={24} pointerEvents="none"/>
                                            </button>
                                        </span> }
                                        {this.props.config.boardName}
                                    </td>

                                    <td style={{ fontWeight: 'bold' }}>Dongle IP address:</td>
                                    <td>
                                        { this.props.withEdit &&
                                        <span data-bs-toggle="modal" data-bs-target="#modalOwnIp">
                                            <this.modalOwnIp />
                                            <button type="button" className="btn btn-outline-secondary p-1 m-1"
                                            data-bs-toggle="tooltip" data-bs-placement="bottom"
                                            title="Edit">
                                                <Icon.Pencil width={24} height={24} pointerEvents="none"/>
                                            </button>
                                        </span> }
                                        {this.props.config.ownIp}
                                    </td>

                                    <td style={{ fontWeight: 'bold' }}>Serial number:</td>
                                    <td>{this.props.config.serial}</td>
                                </tr>
                                <tr>
                                    <td style={{ fontWeight: 'bold' }}>Config source:</td>
                                    <td>{this.props.config.configSource}</td>

                                    <td style={{ fontWeight: 'bold' }}>IP address assigned to host:</td>
                                    <td>{this.props.config.hostIp}</td>

                                    <td style={{ fontWeight: 'bold' }}>Wireless module available:</td>
                                    <td>
                                        { this.props.config.wirelessModule ? <Icon.CheckSquare width={32} height={32} /> : <Icon.XSquareFill width={32} height={32} /> }
                                        { this.props.withEdit &&
                                        <span data-bs-toggle="modal" data-bs-target="#modalWireless">
                                            &nbsp;
                                            <this.modalWireless />
                                            <button type="button" className="btn btn-outline-secondary p-1 m-1"
                                            data-bs-toggle="tooltip" data-bs-placement="bottom"
                                            title="Config">
                                                <Icon.Pencil width={24} height={24} pointerEvents="none"/>
                                            </button>
                                        </span> }
                                    </td>
                                </tr>
                                <tr>
                                    <td style={{ fontWeight: 'bold' }}>Firmware version:</td>
                                    <td>{this.props.config.version}</td>

                                    <td style={{ fontWeight: 'bold' }}>Network mask:</td>
                                    <td>{this.props.config.ownMask}</td>

                                    <td  style={{ fontWeight: 'bold' }}>Status LED brightness:</td>
                                    <td>
                                        {this.props.config.statusLedBrightness}
                                        &nbsp;&nbsp;
                                        <Slider
                                            axis="x"
                                            xstep={10}
                                            xmin={1}
                                            xmax={255}
                                            x={this.props.config.statusLedBrightness}
                                            onChange={({ x }) => this.setStatusLedBrightness(x)}
                                        />
                                    </td>
                                </tr>
                                {this.props.withEdit && 
                                <tr>
                                    <td colSpan="6" className="text-center">
                                        Manage config on base board:&nbsp;&nbsp;&nbsp;
                                        <button type="button" className="btn btn-outline-secondary"
                                          data-bs-toggle="tooltip" data-bs-placement="bottom"
                                          title="Load configuration from base board"
                                          onClick={this.loadConfig.bind(this, 4)}>
                                            <Icon.JournalArrowUp width={32} height={32} pointerEvents="none"/>
                                        </button>
                                        &nbsp;
                                        <button type="button" className="btn btn-outline-secondary"
                                          data-bs-toggle="tooltip" data-bs-placement="bottom"
                                          title="Save configuration to base board"
                                          onClick={this.saveConfig.bind(this, 4)}>
                                            <Icon.JournalArrowDown width={32} height={32} pointerEvents="none"/>
                                        </button>
                                        &nbsp;
                                        <button type="button" className="btn btn-outline-secondary"
                                          data-bs-toggle="tooltip" data-bs-placement="bottom"
                                          title="Enable the configuration saved on the base board"
                                          onClick={this.enableConfig.bind(this, 4)}>
                                              <Icon.JournalCheck width={32} height={32} pointerEvents="none"/>
                                        </button>
                                        &nbsp;
                                        <button type="button" className="btn btn-outline-secondary"
                                          data-bs-toggle="tooltip" data-bs-placement="bottom"
                                          title="Disable the configuration saved on the base board"
                                          onClick={this.disableConfig.bind(this, 4)}>
                                              <Icon.JournalX width={32} height={32} pointerEvents="none"/>
                                        </button>
                                    </td>
                                </tr>
                                }
                            </tbody>
                        </table>
                    </td>
                </tr>
                <tr>
                    {[...Array(4)].map((value, slot) => {
                        return (
                            <td key={slot} colSpan="4" style={{ textAlign: 'center', border: '2px solid black' }}>
                                Slot&nbsp;{slot} { this.props.ioBoards.boards[slot].exist ? <Icon.CheckSquare width={32} height={32} /> : <Icon.XSquareFill width={32} height={32} /> }<br />
                                Type: { this.props.ioBoards.boards[slot].exist ? this.props.ioBoards.boards[slot].type : '???' }<br />
                                {this.props.withEdit &&
                                <div>
                                    <button type="button" className="btn btn-outline-secondary"
                                      data-bs-toggle="tooltip" data-bs-placement="bottom"
                                      title="Load configuration from this io board"
                                      onClick={this.saveConfig.bind(this, slot)}>
                                        <Icon.JournalArrowUp width={32} height={32} pointerEvents="none"/>
                                    </button>
                                    &nbsp;
                                    <button type="button" className="btn btn-outline-secondary"
                                      data-bs-toggle="tooltip" data-bs-placement="bottom"
                                      title="Save configuration to this io board"
                                      onClick={this.saveConfig.bind(this, slot)}>
                                        <Icon.JournalArrowDown width={32} height={32} pointerEvents="none"/>
                                    </button>
                                    &nbsp;
                                    <button type="button" className="btn btn-outline-secondary"
                                      data-bs-toggle="tooltip" data-bs-placement="bottom"
                                      title="Enable the configuration saved on this io board"
                                      onClick={this.enableConfig.bind(this, slot)}>
                                          <Icon.JournalCheck width={32} height={32} pointerEvents="none"/>
                                    </button>
                                    &nbsp;
                                    <button type="button" className="btn btn-outline-secondary"
                                      data-bs-toggle="tooltip" data-bs-placement="bottom"
                                      title="Disable the configuration saved on this io board"
                                      onClick={this.disableConfig.bind(this, slot)}>
                                          <Icon.JournalX width={32} height={32} pointerEvents="none"/>
                                    </button>
                                </div>
                                }
                            </td>
                        )
                    })}
                </tr>

                <tr>
                    {[...Array(16)].map((value, index) => {
                        let slot = Math.floor(index / 4);
                        let port = index % 4;
                        return (
                            <td key={index} style={{ textAlign: 'center', border: '2px solid black' }}>
                                { (
                                    this.props.ioBoards.boards[slot].exist &&
                                    this.props.ioBoards.boards[slot].ports[port].direction !== "unknown" &&
                                    this.props.ioBoards.boards[slot].ports[port].direction !== ""
                                    ) ?
                                <div>
                                { this.props.ioBoards.boards[slot].ports[port].connector === "xlr_5_female" && <img src="media/icon-xlr-5-female.svg" alt="icon-xlr-5-female" width={32} height={32} ></img> }
                                { this.props.ioBoards.boards[slot].ports[port].connector === "xlr_5_male" && <img src="media/icon-xlr-5-male.svg" alt="icon-xlr-5-male" width={32} height={32}></img> }
                                { this.props.ioBoards.boards[slot].ports[port].connector === "xlr_3_female" && <img src="media/icon-xlr-3-female.svg" alt="icon-xlr-3-female" width={32} height={32}></img> }
                                { this.props.ioBoards.boards[slot].ports[port].connector === "xlr_3_male" && <img src="media/icon-xlr-3-male.svg" alt="icon-xlr-3-male" width={32} height={32}></img> }
                                { this.props.ioBoards.boards[slot].ports[port].connector === "rj45" && <img src="media/icon-rj45.svg" alt="icon-rj45" width={32} height={32}></img> }
                                { this.props.ioBoards.boards[slot].ports[port].connector === "screws" && <img src="media/icon-screws.svg" alt="icon-screws" width={32} height={32}></img> }
                                <br />
                                { this.props.ioBoards.boards[slot].ports[port].direction === "out" && <Icon.ArrowDown width={32} height={32} /> }
                                { this.props.ioBoards.boards[slot].ports[port].direction === "in" && <Icon.ArrowUp width={32} height={32} /> }
                                { this.props.ioBoards.boards[slot].ports[port].direction === "switchable" && <Icon.ArrowDownUp width={32} height={32} /> }
                                </div>
                                :
                                <div><Icon.XSquareFill width={32} height={32} /></div>}
                            </td>
                        )
                    })}
                </tr>
            </tbody>
        </table>
    );
  }
}

export default BoardStatus;
