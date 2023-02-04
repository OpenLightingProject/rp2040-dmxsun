import React from "react";
import {
  HashRouter as Router,
  Routes,
  Route,
  Link
} from "react-router-dom";

import Home from "./Home.js";
import Config from "./Config.js";
import Console from "./Console.js";
import Log from "./Log.js";
import Wireless from "./Wireless.js";

import 'bootstrap/dist/css/bootstrap.min.css';

import '@popperjs/core/dist/cjs/popper-lite'

// Required so that we can initialize the tooltips using the example
// mentioned in the official docs at
// https://getbootstrap.com/docs/5.0/components/tooltips/#example-enable-tooltips-everywhere
window.bootstrap = require('bootstrap/dist/js/bootstrap.min.js');

export default function App() {
  // Have a global, optional URL "prefix" so one can have the App
  // served by localhost during development but talk to the API of
  // a REAL dongle
  window.urlPrefix = '';
  //window.urlPrefix = 'http://169.254.53.1'; // comment line if not used; NEVER COMMIT

  return (
    <Router>
      <div>
        <nav className="navbar navbar-expand-lg navbar-light bg-light">
          <span className="navbar-brand"><img src="media/icon-dmxsun.svg" alt="icon-dmxsun" width={56} height={56} style={{ marginLeft: "20px" }} ></img></span>
          <ul className="navbar-nav mr-auto">
            <li className="nav-item active">
              <Link to="/" className="nav-link">Home</Link>
            </li>
            <li className="nav-item">
              <Link to="/console" className="nav-link">Console</Link>
            </li>
            <li className="nav-item">
              <Link to="/config" className="nav-link">Config</Link>
            </li>
            <li className="nav-item">
              <Link to="/wireless" className="nav-link">Wireless</Link>
            </li>
            <li className="nav-item">
              <Link to="/log" className="nav-link">Log</Link>
            </li>
          </ul>
        </nav>

        {/* A <Routes> looks through its children <Route>s and
            renders the first one that matches the current URL. */}
        <Routes>
          <Route path="/console/*" element={<Console/>} />
          <Route path="/config/*" element={<Config/>} />
          <Route path="/wireless/*" element={<Wireless/>} />
          <Route path="/log/*" element={<Log/>} />
          <Route path="/" element={<Home/>} />
        </Routes>
      </div>
    </Router>
  );
}
